/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkiala <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 15:12:59 by gkiala            #+#    #+#             */
/*   Updated: 2026/04/11 15:13:00 by gkiala           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "CommandHandler.hpp"
#include <arpa/inet.h>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

static std::string sanitizeLogLine(const std::string &line) {
  std::istringstream iss(line);
  std::string command;
  iss >> command;
  for (size_t i = 0; i < command.size(); ++i) {
    command[i] = static_cast<char>(std::toupper(command[i]));
  }
  if (command == "PASS") {
    return "PASS ******";
  }
  return line;
}

Server::Server(int port, const std::string &password)
    : _port(port), _password(password), _listenFd(-1), _running(false) {}

Server::~Server() {
  for (size_t i = 0; i < _pollFds.size(); ++i) {
    if (_pollFds[i].fd >= 0) {
      close(_pollFds[i].fd);
    }
  }
}

void Server::_initSocket() {
  _listenFd = socket(AF_INET, SOCK_STREAM, 0);
  if (_listenFd < 0) {
    throw std::runtime_error("socket() failed");
  }

  int enable = 1;
  if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
    throw std::runtime_error("setsockopt() failed");
  }
  if (fcntl(_listenFd, F_SETFL, O_NONBLOCK) < 0) {
    throw std::runtime_error("fcntl() failed on server socket");
  }

  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(_port);

  if (bind(_listenFd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    throw std::runtime_error("bind() failed");
  }
  if (listen(_listenFd, SOMAXCONN) < 0) {
    throw std::runtime_error("listen() failed");
  }

  pollfd pfd;
  pfd.fd = _listenFd;
  pfd.events = POLLIN;
  pfd.revents = 0;
  _pollFds.push_back(pfd);
}

void Server::start() {
  _initSocket();
  _running = true;
  std::ostringstream startup;
  startup << "Server started on port " << _port;
  logAction(startup.str());

  while (_running) {
    int ready = poll(&_pollFds[0], _pollFds.size(), -1);
    if (ready < 0) {
      throw std::runtime_error("poll() failed");
    }
    for (size_t i = 0; i < _pollFds.size() && ready > 0; ++i) {
      if (_pollFds[i].revents == 0) {
        continue;
      }
      --ready;
      if (_pollFds[i].fd == _listenFd && (_pollFds[i].revents & POLLIN)) {
        _acceptClient();
      } else {
        _handleClientInput(i);
        if (i < _pollFds.size()) {
          _handleClientOutput(i);
        }
      }
    }
  }
}

void Server::stop() { _running = false; }

void Server::_acceptClient() {
  sockaddr_in addr;
  socklen_t len = sizeof(addr);
  int clientFd = accept(_listenFd, reinterpret_cast<sockaddr *>(&addr), &len);
  if (clientFd < 0) {
    return;
  }
  if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0) {
    close(clientFd);
    return;
  }

  std::string host = inet_ntoa(addr.sin_addr);
  _clients.insert(std::make_pair(clientFd, Client(clientFd, host)));
  logAction("Client connected: " + host);

  pollfd pfd;
  pfd.fd = clientFd;
  pfd.events = POLLIN;
  pfd.revents = 0;
  _pollFds.push_back(pfd);
}

void Server::_setPollWriteEnabled(int fd, bool enabled) {
  for (size_t i = 0; i < _pollFds.size(); ++i) {
    if (_pollFds[i].fd == fd) {
      if (enabled) {
        _pollFds[i].events |= POLLOUT;
      } else {
        _pollFds[i].events &= static_cast<short>(~POLLOUT);
      }
      return;
    }
  }
}

void Server::_handleClientInput(size_t pollIndex) {
  int fd = _pollFds[pollIndex].fd;
  if (_pollFds[pollIndex].revents & (POLLHUP | POLLERR | POLLNVAL)) {
    _disconnectClient(fd, "Connection closed");
    return;
  }
  if (!(_pollFds[pollIndex].revents & POLLIN)) {
    return;
  }

  char buffer[512];
  std::memset(buffer, 0, sizeof(buffer));
  ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
  if (bytes <= 0) {
    _disconnectClient(fd, "Client disconnected");
    return;
  }

  Client *client = getClient(fd);
  if (client == NULL) {
    _disconnectClient(fd, "Unknown client");
    return;
  }
  client->appendRecvData(std::string(buffer, bytes));
  if (client->shouldDisconnect()) {
    _disconnectClient(fd, "Receive buffer exceeded");
    return;
  }

  std::string line;
  while (client->popNextLine(line)) {
    if (line.empty()) {
      continue;
    }
    const std::string safeLine = sanitizeLogLine(line);
    if (client->getNickname().empty()) {
      logAction("Command received: " + safeLine);
    } else {
      logAction(client->getNickname() + " sent: " + safeLine);
    }
    handleLine(*client, line);
    client = getClient(fd);
    if (client == NULL) {
      return;
    }
  }
}

void Server::_handleClientOutput(size_t pollIndex) {
  int fd = _pollFds[pollIndex].fd;
  if (_pollFds[pollIndex].revents & (POLLHUP | POLLERR | POLLNVAL)) {
    return;
  }
  if (!(_pollFds[pollIndex].revents & POLLOUT)) {
    return;
  }

  Client *client = getClient(fd);
  if (client == NULL) {
    _disconnectClient(fd, "Unknown client");
    return;
  }
  if (!client->hasPendingSend()) {
    _setPollWriteEnabled(fd, false);
    return;
  }

  const std::string &out = client->getSendBuffer();
  ssize_t sent = send(fd, out.c_str(), out.size(), 0);
  if (sent <= 0) {
    _disconnectClient(fd, "Send failed");
    return;
  }
  client->consumeSentBytes(static_cast<size_t>(sent));
  if (!client->hasPendingSend()) {
    _setPollWriteEnabled(fd, false);
  } else {
    _setPollWriteEnabled(fd, true);
  }
}

void Server::_disconnectClient(int fd, const std::string &reason) {
  Client *client = getClient(fd);
  if (client != NULL) {
    if (!client->getNickname().empty()) {
      logAction(client->getNickname() + " disconnected (" + reason + ")");
    } else {
      logAction("Client disconnected (" + reason + ")");
    }
    removeClientFromAllChannels(*client, reason);
    _clients.erase(fd);
  } else {
    logAction("Connection closed (" + reason + ")");
  }
  _removePollFd(fd);
  close(fd);
}

void Server::_removePollFd(int fd) {
  for (size_t i = 0; i < _pollFds.size(); ++i) {
    if (_pollFds[i].fd == fd) {
      _pollFds.erase(_pollFds.begin() + i);
      return;
    }
  }
}

const std::string &Server::getPassword() const { return _password; }

std::string Server::getServerName() const { return "ircserv.local"; }

Client *Server::getClient(int fd) {
  std::map<int, Client>::iterator it = _clients.find(fd);
  if (it == _clients.end()) {
    return NULL;
  }
  return &it->second;
}

const Client *Server::getClient(int fd) const {
  std::map<int, Client>::const_iterator it = _clients.find(fd);
  if (it == _clients.end()) {
    return NULL;
  }
  return &it->second;
}

Client *Server::getClientByNick(const std::string &nickname) {
  for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
    if (it->second.getNickname() == nickname) {
      return &it->second;
    }
  }
  return NULL;
}

bool Server::isNicknameInUse(const std::string &nickname, int exceptFd) const {
  for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
    if (it->first != exceptFd && it->second.getNickname() == nickname) {
      return true;
    }
  }
  return false;
}

bool Server::isValidNickname(const std::string &nickname) {
  if (nickname.empty() || nickname.size() > 30) {
    return false;
  }
  if (!(std::isalpha(nickname[0]) || std::string("[]\\`_^{|}").find(nickname[0]) != std::string::npos)) {
    return false;
  }
  for (size_t i = 1; i < nickname.size(); ++i) {
    const char c = nickname[i];
    if (!std::isalnum(c) && std::string("-[]\\`_^{|}").find(c) == std::string::npos) {
      return false;
    }
  }
  return true;
}

Channel *Server::getChannel(const std::string &channelName) {
  std::map<std::string, Channel>::iterator it = _channels.find(channelName);
  if (it == _channels.end()) {
    return NULL;
  }
  return &it->second;
}

const Channel *Server::getChannel(const std::string &channelName) const {
  std::map<std::string, Channel>::const_iterator it = _channels.find(channelName);
  if (it == _channels.end()) {
    return NULL;
  }
  return &it->second;
}

Channel &Server::getOrCreateChannel(const std::string &channelName) {
  std::map<std::string, Channel>::iterator it = _channels.find(channelName);
  if (it == _channels.end()) {
    _channels.insert(std::make_pair(channelName, Channel(channelName)));
  }
  return _channels.find(channelName)->second;
}

void Server::eraseChannelIfEmpty(const std::string &channelName) {
  Channel *channel = getChannel(channelName);
  if (channel != NULL && channel->empty()) {
    _channels.erase(channelName);
  }
}

void Server::sendToClient(int fd, const std::string &message) {
  if (message.empty()) {
    return;
  }
  Client *client = getClient(fd);
  if (client == NULL) {
    return;
  }
  client->queueSendData(message);
  _setPollWriteEnabled(fd, true);
}

void Server::sendNumeric(const Client &client, int code, const std::string &message) const {
  std::ostringstream oss;
  oss << ":" << getServerName() << " ";
  if (code < 100) {
    oss << "0";
  }
  if (code < 10) {
    oss << "0";
  }
  oss << code << " ";
  if (client.getNickname().empty()) {
    oss << "*";
  } else {
    oss << client.getNickname();
  }
  oss << " :" << message << "\r\n";
  const_cast<Server *>(this)->sendToClient(client.getFd(), oss.str());
}

void Server::broadcastChannel(const std::string &channelName, const std::string &message, int exceptFd) {
  Channel *channel = getChannel(channelName);
  if (channel == NULL) {
    return;
  }
  const std::set<int> &members = channel->getMembers();
  for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
    if (*it == exceptFd) {
      continue;
    }
    sendToClient(*it, message);
  }
}

void Server::handleLine(Client &client, const std::string &line) {
  CommandHandler::handle(*this, client, line);
}

void Server::_registerIfReady(Client &client) {
  if (client.isRegistered()) {
    return;
  }
  if (client.isPassAccepted() && client.hasNick() && client.hasUser()) {
    client.setRegistered(true);
    logAction("Client registered: " + client.getNickname());
    sendNumeric(client, 1, "Welcome to ft_irc " + client.getNickname());
    sendNumeric(client, 2, "Your host is " + getServerName());
    sendNumeric(client, 3, "This server was created for ft_irc");
    sendToClient(client.getFd(), ":" + getServerName() + " 004 " + client.getNickname() + " " + getServerName() + " ft_irc-1.0 o itkol\r\n");
  }
}

void Server::removeClientFromAllChannels(Client &client, const std::string &reason) {
  std::set<std::string> joined = client.getJoinedChannels();
  std::string quitMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHost();
  quitMsg += " QUIT :" + reason + "\r\n";

  for (std::set<std::string>::const_iterator it = joined.begin(); it != joined.end(); ++it) {
    Channel *channel = getChannel(*it);
    if (channel == NULL) {
      continue;
    }
    broadcastChannel(*it, quitMsg, client.getFd());
    channel->removeMember(client.getFd());
    eraseChannelIfEmpty(*it);
  }
}

void Server::disconnectClient(int fd, const std::string &reason) { _disconnectClient(fd, reason); }

void Server::logAction(const std::string &message) const { std::cout << "[IRC] " << message << std::endl; }

void Server::tryRegisterClient(Client &client) { _registerIfReady(client); }