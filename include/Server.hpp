/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkiala <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 14:10:18 by gkiala            #+#    #+#             */
/*   Updated: 2026/04/11 14:10:20 by gkiala           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Channel.hpp"
#include "Client.hpp"
#include <map>
#include <poll.h>
#include <string>
#include <vector>

class Server {
  private:
    int _port;
    std::string _password;
    int _listenFd;
    std::vector<pollfd> _pollFds;
    std::map<int, Client> _clients;
    std::map<std::string, Channel> _channels;
    bool _running;

    Server(const Server &other);
    Server &operator=(const Server &other);

    void _initSocket();
    void _acceptClient();
    void _handleClientInput(size_t pollIndex);
    void _handleClientOutput(size_t pollIndex);
    void _setPollWriteEnabled(int fd, bool enabled);
    void _disconnectClient(int fd, const std::string &reason);
    void _removePollFd(int fd);
    void _registerIfReady(Client &client);

  public:
    Server(int port, const std::string &password);
    ~Server();

    void start();
    void stop();

    const std::string &getPassword() const;
    std::string getServerName() const;

    Client *getClient(int fd);
    const Client *getClient(int fd) const;
    Client *getClientByNick(const std::string &nickname);
    bool isNicknameInUse(const std::string &nickname, int exceptFd) const;
    static bool isValidNickname(const std::string &nickname);

    Channel *getChannel(const std::string &channelName);
    const Channel *getChannel(const std::string &channelName) const;
    Channel &getOrCreateChannel(const std::string &channelName);
    void eraseChannelIfEmpty(const std::string &channelName);

    void sendToClient(int fd, const std::string &message);
    void sendNumeric(const Client &client, int code, const std::string &message) const;
    void broadcastChannel(const std::string &channelName, const std::string &message, int exceptFd);
    void handleLine(Client &client, const std::string &line);

    void removeClientFromAllChannels(Client &client, const std::string &reason);
    void disconnectClient(int fd, const std::string &reason);
    void logAction(const std::string &message) const;
    void tryRegisterClient(Client &client);
};

#endif