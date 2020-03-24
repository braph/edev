#include "trackloader.hpp"

#include "lib/downloads.hpp"
#include "lib/filesystem.hpp"
#include "config.hpp"
#include "ektoplayer.hpp"

#include <iostream>

TrackLoader :: TrackLoader(Downloads& downloads)
: downloads(downloads)
{
}

std::string TrackLoader :: getFileForTrack(Database::Tracks::Track track, bool force_download) {
#if 0
  std::string archive_mp3_url = track.archive_mp3_url();

  if (archive_mp3_url.empty()) {
    Filesystem::path archive = archive_mp3_url;

    archive = archive.filename().stem();
    Filesystem::path full_path = Config::archive_dir;
    full_path /= archive;

    //album_files = Dir.glob(File.join(track_info['album_path'], '*.mp3'))
    //track_file = album_files.sort[track_info['number']]
    //return track_file if track_file
  }
#endif

  std::cerr << "Track " << track.title();

  std::string track_url = track.url();
  Filesystem::path track_file = track_url;
  track_file += ".mp3";

  // $cache_dir/$track_file
  Filesystem::path file_in_cache = Config::cache_dir;
  file_in_cache /= track_file;

  // $temp_dir/~ekto-$track_file
  Filesystem::path file_in_temp = Config::temp_dir;
  file_in_temp /= EKTOPLAZM_TEMP_FILE_PREFIX;
  file_in_temp += track_file;

  if (force_download) {
    Filesystem::error_code e;
    Filesystem::remove(file_in_temp, e);
    Filesystem::remove(file_in_cache, e);
  }

  if (Filesystem::exists(file_in_temp)) {
    std::cerr << " -> TEMP: " << file_in_temp << '\n';
    return file_in_temp.string();
  }

  if (Filesystem::exists(file_in_cache)) {
    std::cerr << " -> CACHE: " << file_in_cache << '\n';
    return file_in_cache.string();
  }

  Ektoplayer::url_expand(track_url, EKTOPLAZM_TRACK_BASE_URL, ".mp3");
  std::cerr << " -> DOWNLOAD: " << track_url << '\n';

  FileDownload* fileDownload = new FileDownload(track_url, file_in_temp.string());
  fileDownload->onFinished = [=](Download& _dl, CURLcode curl_e) {
    FileDownload& dl = static_cast<FileDownload&>(_dl);
    Filesystem::error_code e;
    if (curl_e == CURLE_OK && dl.httpCode() == 200) {
      if (Config::use_cache) {
        Filesystem::rename(dl.filename(), file_in_cache, e);
        if (e) {
          Filesystem::copy(dl.filename(), file_in_cache, e);
          Filesystem::remove(dl.filename(), e);
        }
      }
    } else {
      Filesystem::remove(dl.filename(), e);
    }
    std::cerr << dl.lastURL() << ": " << curl_easy_strerror(curl_e) << " [" << dl.httpCode() << "]\n";
  };

  downloads.addDownload(fileDownload, Downloads::HIGH);
  return fileDownload->filename();
}

#if 0
void TrackLoader :: download_album(const Database::Tracks::Track& track) {
         track_info = get_track_infos(url)
         return if File.exists? track_info['album_path']

         archive_file = File.join(Config[:download_dir], track_info['archive_filename'])
         return if File.exists? archive_file

         archive_url = Application.archive_url(track_info['archive_url'])
         Application.log(self, 'starting download:', archive_url)
         dl = DownloadThread.new(archive_url, archive_file)

         if Config[:auto_extract_to_archive_dir]
            dl.events.on(:completed) do 
               begin
                  extract_dir = track_info['album_path']
                  FileUtils::mkdir_p(extract_dir)
                  Common::extract_zip(archive_file, extract_dir)
                  FileUtils::rm(archive_file) if Config[:delete_after_extraction]
               rescue
                  Application.log(self, "extraction of '#{archive_file}' to '#{extract_dir}' failed:", $!)
               end
            end
         end

         dl.events.on(:failed) do |reason|
            Application.log(self, dl.filename, dl.url, reason)
            FileUtils::rm(dl.filename) rescue nil
         end

         @downloads << dl.start!
      rescue
         Application.log(self, $!)
      end
}
#endif
