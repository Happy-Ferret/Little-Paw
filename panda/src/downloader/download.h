// Filename: download.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef DOWNLOAD_H
#define DOWNLOAD_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <notify.h>
#include <filename.h>
#include <buffer.h>
#include <pointerTo.h>
#include <clockObject.h>

#if defined(WIN32_VC)
  #include <winsock2.h>
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <resolv.h>
#endif

////////////////////////////////////////////////////////////////////
//       Class : Download
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Download {
PUBLISHED:
  enum DownloadCode {
    DS_ok = 3,
    DS_write = 2,
    DS_success = 1,
    DS_error = -1,
    DS_error_write = -2,
    DS_error_connect = -3,
  };

  Download(void);
  virtual ~Download(void);

  bool connect_to_server(const string &name, uint port=80);
  void disconnect_from_server(void);

  int initiate(const string &file_name, Filename file_dest);
  int initiate(const string &file_name, Filename file_dest,
		int first_byte, int last_byte, int total_bytes,
		bool partial_content = true);
  int run(void);

  INLINE void set_frequency(float frequency);
  INLINE float get_frequency(void) const;
  INLINE void set_byte_rate(float bytes);
  INLINE float get_byte_rate(void) const;
  INLINE void set_disk_write_frequency(int frequency);
  INLINE int get_disk_write_frequency(void) const;
  INLINE int get_bytes_written(void) const;
  INLINE float get_bytes_per_second(void) const;

private:
  class DownloadStatus {
  public:
    DownloadStatus(char *buffer, int first_byte, int last_byte,
			int total_bytes, bool partial_content);
    void reset(void);

  public:
    bool _first_line_complete;
    bool _header_is_complete;
    bool _header_is_valid;
    char *_start;
    char *_next_in;
    int _bytes_in_buffer;
    int _total_bytes_written;
    int _first_byte;
    int _last_byte;
    int _total_bytes;
    bool _partial_content;
    char *_buffer;
  };

  INLINE void recompute_buffer(void);

  bool connect_to_server(void);
  int safe_send(int socket, const char *data, int length, long timeout);
  int fast_receive(int socket, DownloadStatus *status, int rec_size);
  bool parse_http_response(const string &resp);
  bool parse_header(DownloadStatus *status);
  bool write_to_disk(DownloadStatus *status);

private:
  bool _connected;
  int _socket;
  string _server_name;
  struct sockaddr_in _sin;

  PT(Buffer) _buffer;
  int _disk_write_frequency;
  float _frequency;
  float _byte_rate; 
  int _receive_size;
  int _disk_buffer_size;
  ofstream _dest_stream;
  bool _recompute_buffer;

  DownloadStatus *_current_status;
  bool _got_any_data;

  double _tlast;
  double _tfirst;
  ClockObject _clock;
};

#include "download.I"

#endif
