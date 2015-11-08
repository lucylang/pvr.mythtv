/*
 *      Copyright (C) 2014 Jean-Luc Barriere
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef MYTHSOCKET_H
#define	MYTHSOCKET_H

#include "os/os.h"

#include <cstddef>  // for size_t
#include <string>

#define SOCKET_HOSTNAME_MAXSIZE       1025
#define SOCKET_RCVBUF_MINSIZE         16384
#define SOCKET_READ_TIMEOUT_SEC       10
#define SOCKET_READ_TIMEOUT_USEC      0
#define SOCKET_READ_ATTEMPT           3
#define SOCKET_BUFFER_SIZE            1472

namespace Myth
{

  typedef enum {
    SOCKET_AF_INET4,
    SOCKET_AF_INET6,
  } SOCKET_AF_t;

  struct SocketAddress;

  class NetSocket
  {
  public:
    NetSocket()
    {
      m_timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
      m_timeout.tv_usec = SOCKET_READ_TIMEOUT_USEC;
    }
    virtual ~NetSocket() { }
    virtual bool SendData(const char* buf, size_t size) = 0;
    virtual size_t ReceiveData(void* buf, size_t n) = 0;
    void SetTimeout(timeval timeout)
    {
      m_timeout = timeout;
    }

  protected:
    struct timeval m_timeout;
  };

  class TcpSocket : public NetSocket
  {
    friend class TcpServerSocket;
  public:
    TcpSocket();
    ~TcpSocket();

    // Implements NetSocket
    bool SendData(const char* buf, size_t size)
    {
      return SendMessage(buf, size);
    }
    size_t ReceiveData(void* buf, size_t n)
    {
      return ReadResponse(buf, n);
    }

    int GetErrNo() const
    {
      return m_errno;
    }
    bool Connect(const char *server, unsigned port, int rcvbuf);
    bool SendMessage(const char *msg, size_t size);
    void SetReadAttempt(int n)
    {
      m_attempt = n;
    }
    size_t ReadResponse(void *buf, size_t n);
    void Disconnect();
    bool IsValid() const
    {
      return (m_socket == INVALID_SOCKET_VALUE ? false : true);
    }
    bool IsConnected() const
    {
      return IsValid();
    }
    int Listen(timeval *timeout);
    net_socket_t GetSocket() const;
    std::string GetLocalIP();

    static const char* GetMyHostName();

  private:
    net_socket_t m_socket;
    int m_rcvbuf;
    int m_errno;
    int m_attempt;
    char* m_buffer;
    char* m_bufptr;
    size_t m_buflen;
    size_t m_rcvlen;

    // prevent copy
    TcpSocket(const TcpSocket&);
    TcpSocket& operator=(const TcpSocket&);
  };

  class TcpServerSocket
  {
  public:
    TcpServerSocket();
    ~TcpServerSocket();

    int GetErrNo() const
    {
      return m_errno;
    }
    bool Create(SOCKET_AF_t af);
    bool IsValid() const
    {
      return (m_socket == INVALID_SOCKET_VALUE ? false : true);
    }
    bool Bind(unsigned port);
    bool ListenConnection();
    bool AcceptConnection(TcpSocket& socket);
    void Close();

  private:
    SocketAddress* m_addr;
    net_socket_t m_socket;
    int m_errno;
    unsigned m_maxconnections;

    // prevent copy
    TcpServerSocket(const TcpServerSocket&);
    TcpServerSocket& operator=(const TcpServerSocket&);
  };

  class UdpSocket : public NetSocket
  {
  public:
    UdpSocket();
    UdpSocket(size_t bufferSize);
    ~UdpSocket();

    // Implements NetSocket
    bool SendData(const char* data, size_t size);
    size_t ReceiveData(void* buf, size_t n);

    int GetErrNo() const
    {
      return m_errno;
    }
    bool SetAddress(SOCKET_AF_t af, const char *target, unsigned port);
    bool SetMulticastTTL(int multicastTTL);
    size_t GetPayloadLength() const
    {
      return m_rcvlen;
    }
    void FlushPayload()
    {
      m_rcvlen = 0;
    }
    std::string GetRemoteIP() const;
    bool IsValid() const
    {
      return (m_socket == INVALID_SOCKET_VALUE ? false : true);
    }

  private:
    SocketAddress* m_addr;
    SocketAddress* m_from;
    net_socket_t m_socket;
    int m_errno;
    char* m_buffer;
    char* m_bufptr;
    size_t m_buflen;
    size_t m_rcvlen;

    // prevent copy
    UdpSocket(const UdpSocket&);
    UdpSocket& operator=(const UdpSocket&);
  };

}

#endif	/* MYTHSOCKET_H */
