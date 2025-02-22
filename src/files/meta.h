#pragma once
#include <QVariant>
extern "C" {
#include <libavformat/avformat.h>
}

QVariantMap getMediaMeta(const QString& path);
