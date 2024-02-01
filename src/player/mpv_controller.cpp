#include <QDebug>
#include <QVariant>

#include <clocale>

#include "mpv_controller.h"

struct node_autofree {
	mpv_node *ptr;
	explicit node_autofree(mpv_node *a_ptr)
		: ptr(a_ptr) {}
	~node_autofree() {
		mpv_free_node_contents(ptr);
	}
};

inline bool test_type(const QVariant &v, QMetaType::Type t) {
	return static_cast<int>(v.type()) == static_cast<int>(t);
}

inline void free_node(mpv_node *dst) {
	switch (dst->format) {
	case MPV_FORMAT_STRING:
		delete[] dst->u.string;
		break;
	case MPV_FORMAT_NODE_ARRAY:
	case MPV_FORMAT_NODE_MAP: {
		mpv_node_list *list = dst->u.list;
		if (list) {
			for (int n = 0; n < list->num; n++) {
				if (list->keys) {
					delete[] list->keys[n];
				}
				if (list->values) {
					free_node(&list->values[n]);
				}
			}
			delete[] list->keys;
			delete[] list->values;
		}
		delete list;
		break;
	}
	default:;
	}
	dst->format = MPV_FORMAT_NONE;
}

inline mpv_node_list *create_list(mpv_node *dst, bool is_map, int num) {
	dst->format = is_map ? MPV_FORMAT_NODE_MAP : MPV_FORMAT_NODE_ARRAY;
	mpv_node_list *list = new mpv_node_list();
	dst->u.list = list;
	if (!list) {
		free_node(dst);
		return nullptr;
	}
	list->values = new mpv_node[num]();
	if (!list->values) {
		free_node(dst);
		return nullptr;
	}
	if (is_map) {
		list->keys = new char *[num]();
		if (!list->keys) {
			free_node(dst);
			return nullptr;
		}
	}
	return list;
}

inline QVariant node_to_variant(const mpv_node *node) {
	switch (node->format) {
	case MPV_FORMAT_STRING:
		return QVariant(QString::fromUtf8(node->u.string));
	case MPV_FORMAT_FLAG:
		return QVariant(static_cast<bool>(node->u.flag));
	case MPV_FORMAT_INT64:
		return QVariant(static_cast<qlonglong>(node->u.int64));
	case MPV_FORMAT_DOUBLE:
		return QVariant(node->u.double_);
	case MPV_FORMAT_NODE_ARRAY: {
		mpv_node_list *list = node->u.list;
		QVariantList qlist;
		for (int n = 0; n < list->num; n++)
			qlist.append(node_to_variant(&list->values[n]));
		return QVariant(qlist);
	}
	case MPV_FORMAT_NODE_MAP: {
		mpv_node_list *list = node->u.list;
		QVariantMap qmap;
		for (int n = 0; n < list->num; n++) {
			qmap.insert(QString::fromUtf8(list->keys[n]), node_to_variant(&list->values[n]));
		}
		return QVariant(qmap);
	}
	default: // MPV_FORMAT_NONE, unknown values (e.g. future extensions)
		return QVariant();
	}
}

void setNode(mpv_node *dst, const QVariant &src) {
	if (test_type(src, QMetaType::QString)) {
		dst->format = MPV_FORMAT_STRING;
		dst->u.string = qstrdup(src.toString().toUtf8().data());
		if (!dst->u.string) {
			dst->format = MPV_FORMAT_NONE;
		}
	} else if (test_type(src, QMetaType::Bool)) {
		dst->format = MPV_FORMAT_FLAG;
		dst->u.flag = src.toBool() ? 1 : 0;
	} else if (test_type(src, QMetaType::Int) || test_type(src, QMetaType::LongLong) || test_type(src, QMetaType::UInt) || test_type(src, QMetaType::ULongLong)) {
		dst->format = MPV_FORMAT_INT64;
		dst->u.int64 = src.toLongLong();
	} else if (test_type(src, QMetaType::Double)) {
		dst->format = MPV_FORMAT_DOUBLE;
		dst->u.double_ = src.toDouble();
	} else if (src.canConvert<QVariantList>()) {
		QVariantList qlist = src.toList();
		mpv_node_list *list = create_list(dst, false, qlist.size());
		if (!list) {
			dst->format = MPV_FORMAT_NONE;
			return;
		}
		list->num = qlist.size();
		for (int n = 0; n < qlist.size(); n++)
			setNode(&list->values[n], qlist[n]);
	} else if (src.canConvert<QVariantMap>()) {
		QVariantMap qmap = src.toMap();
		mpv_node_list *list = create_list(dst, true, qmap.size());
		if (!list) {
			dst->format = MPV_FORMAT_NONE;
			return;
		}
		list->num = qmap.size();
		int n = 0;
		for (auto it = qmap.constKeyValueBegin(); it != qmap.constKeyValueEnd(); ++it) {
			list->keys[n] = qstrdup(it.operator*().first.toUtf8().data());
			if (!list->keys[n]) {
				free_node(dst);
				dst->format = MPV_FORMAT_NONE;
				return;
			}
			setNode(&list->values[n], it.operator*().second);
			++n;
		}
	} else {
		dst->format = MPV_FORMAT_NONE;
	}
	return;
}

dp::player::MpvController::MpvController(bool verbose, QObject *parent): QObject(parent) {
	std::setlocale(LC_NUMERIC, "C");

	m_mpv = mpv_create();
	if (!m_mpv) {
		qFatal("could not create mpv context");
	}
	if (mpv_initialize(m_mpv) < 0) {
		qFatal("could not initialize mpv context");
	}
	mpv_set_wakeup_callback(m_mpv, MpvController::mpvEvents, this);

	if (verbose) {
		mpv_set_option_string(m_mpv, "terminal", "yes");
		mpv_set_option_string(m_mpv, "msg-level", "all=v");
	}
}

void dp::player::MpvController::mpvEvents(void *ctx) {
	QMetaObject::invokeMethod(static_cast<MpvController *>(ctx), &MpvController::eventHandler, Qt::QueuedConnection);
}

mpv_handle *dp::player::MpvController::mpv() const {
	return m_mpv;
}

QVariant dp::player::MpvController::setProperty(const QString &property, const QVariant &value) {
	mpv_node node;
	setNode(&node, value);
	int err = mpv_set_property(m_mpv, property.toUtf8().constData(), MPV_FORMAT_NODE, &node);
	if (err < 0) {
		return QVariant::fromValue(MpvErrorType(mpv_error_string(err)));
	}
	return QVariant();
}

QVariant dp::player::MpvController::getProperty(const QString &property) {
	mpv_node node;
	int err = mpv_get_property(m_mpv, property.toUtf8().constData(), MPV_FORMAT_NODE, &node);
	if (err < 0) {
		return QVariant::fromValue(MpvErrorType(mpv_error_string(err)));
	}
	node_autofree f(&node);
	return node_to_variant(&node);
}

QVariant dp::player::MpvController::command(const QVariant &params) {
	mpv_node node;
	setNode(&node, params);
	mpv_node result;
	int err = mpv_command_node(m_mpv, &node, &result);
	if (err < 0) {
		return QVariant::fromValue(MpvErrorType(mpv_error_string(err)));
	}
	node_autofree f(&result);
	return node_to_variant(&result);
}

Q_INVOKABLE void dp::player::MpvController::observeProperty(const QString &property, const mpv_format format) {
	mpv_observe_property(m_mpv, 0, qstrdup(property.toUtf8().data()), format);
}

void dp::player::MpvController::eventHandler() {
	while (m_mpv) {
		mpv_event *event = mpv_wait_event(m_mpv, 0);
		if (event->event_id == MPV_EVENT_NONE) {
			break;
		}
		switch (event->event_id) {
		case MPV_EVENT_FILE_LOADED: {
			Q_EMIT tracksLoaded();
			break;
		}
		case MPV_EVENT_END_FILE: {
			auto prop = static_cast<mpv_event_end_file*>(event->data);
			if (prop->reason == MPV_END_FILE_REASON_EOF) {
				Q_EMIT endFile();
			} else if (prop->reason == MPV_END_FILE_REASON_ERROR) {
				Q_EMIT endError(mpv_error_string(prop->error));
			}
			break;
		}
		case MPV_EVENT_PROPERTY_CHANGE: {
			mpv_event_property *prop = static_cast<mpv_event_property *>(event->data);
			QVariant data;
			switch (prop->format) {
			case MPV_FORMAT_DOUBLE:
				data = *reinterpret_cast<double *>(prop->data);
				break;
			case MPV_FORMAT_STRING:
				data = QString::fromStdString(*reinterpret_cast<char **>(prop->data));
				break;
			case MPV_FORMAT_INT64:
				data = qlonglong(*reinterpret_cast<int64_t *>(prop->data));
				break;
			case MPV_FORMAT_FLAG:
				data = *reinterpret_cast<bool *>(prop->data);
				break;
			case MPV_FORMAT_NODE:
				data = node_to_variant(reinterpret_cast<mpv_node *>(prop->data));
				break;
			default:
				break;
			}
			Q_EMIT propertyChanged(QString::fromStdString(prop->name), data);
			break;
		}
		default:
			qDebug() << "Event default: " << mpv_event_name(event->event_id);
			// Ignore uninteresting or unknown events.
		}
	}
}
