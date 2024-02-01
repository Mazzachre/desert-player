CREATE TABLE files (path TEXT PRIMARY KEY NOT NULL, media TEXT NOT NULL, tracks TEXT, playback TEXT);
CREATE TABLE tmdbId (path TEXT PRIMARY KEY NOT NULL, tmdbid INT, tmdbtype TEXT, season INT, episode INT);
CREATE TABLE playlists (label TEXT NOT NULL);
CREATE TABLE playlistFiles (playlist INT NOT NULL, file TEXT NOT NULL);

CREATE INDEX idx_movie ON tmdbId(tmdbid, tmdbtype);
CREATE INDEX idx_tv_show ON tmdbId(tmdbid, tmdbtype, season, episode);
CREATE UNIQUE INDEX idx_unique_playlist ON playlistFiles (playlist, file);
