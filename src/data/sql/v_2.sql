ALTER TABLE playlists ADD COLUMN files TEXT;

UPDATE playlists SET files = (SELECT GROUP_CONCAT(files.rowid) FROM files JOIN playlistFiles ON files.path = playlistFiles.file WHERE playlistFiles.playlist = playlists.rowid ORDER BY files.rowid);

DROP TABLE playlistFiles;
