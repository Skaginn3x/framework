#pragma once
#define ASIO_STANDALONE  // sorry, Boost-ified Asio is not yet supported

#if defined(_WIN32) and __has_include(<SDKDDKVer.h>)
#include <SDKDDKVer.h>
#endif

#if defined(ASIO_HEADER_ONLY)
#error "ASIO_HEADER_ONLY" makes no sense with this module
#endif

#if !defined(ASIO_SEPARATE_COMPILATION)
#define ASIO_SEPARATE_COMPILATION
#endif

#if !defined(ASIO_DISABLE_BUFFER_DEBUGGING) && !defined(ASIO_ENABLE_BUFFER_DEBUGGING)
#define ASIO_DISABLE_BUFFER_DEBUGGING
#endif

#define ASIO_NO_DEPRECATED
#define ASIO_MODULE

#include <asio/detail/config.hpp>

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <climits>
#include <compare>
#include <concepts>
#include <condition_variable>
#include <coroutine>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <exception>
#include <forward_list>
#include <functional>
#include <future>
#include <ios>
#include <iosfwd>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <ostream>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <version>

#include <sys/stat.h>
#include <sys/types.h>

#if defined(ASIO_WINDOWS_RUNTIME)  // expected to not work at all
#include <robuffer.h>
#include <windows.storage.streams.h>
#include <wrl/implements.h>
#include <codecvt>
#include <locale>
#elif defined(ASIO_WINDOWS) or defined(__CYGWIN__)
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
//  do not reorder!
#include <MSWSock.h>
#include <process.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <termios.h>
#endif

#ifdef ASIO_HAS_PTHREADS
#include <pthread.h>
#endif
#if defined(ASIO_HAS_IO_URING)
#include <liburing.h>
#endif
#if defined(ASIO_HAS_KQUEUE)
#include <sys/event.h>
#include <sys/time.h>
#endif
#if defined(ASIO_HAS_DEV_POLL)
#include <sys/devpoll.h>
#endif
#if defined(ASIO_HAS_EPOLL)
#include <sys/epoll.h>
#if defined(ASIO_HAS_TIMERFD)
#include <sys/timerfd.h>
#endif
#endif
#if defined(ASIO_HAS_EVENTFD)
#include <sys/eventfd.h>
#endif
#if defined(ASIO_HAS_PIPE) and defined(ASIO_HAS_IOCP)
#include <bcrypt.h>
#endif

#ifdef ASIO_ENABLE_HANDLER_TRACKING
#include <cstdarg>
#endif

#if defined(ASIO_USE_SSL)
#if defined(ASIO_USE_WOLFSSL)
#include <wolfssl/options.h>
#endif  // defined(ASIO_USE_WOLFSSL)

#include <openssl / conf.h>
#include <openssl/ssl.h>
#if !defined(OPENSSL_NO_ENGINE)
#include <openssl/engine.h>
#endif  // !defined(OPENSSL_NO_ENGINE)
#include <openssl/dh.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#endif
