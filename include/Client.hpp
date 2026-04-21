/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkiala <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 14:10:32 by gkiala            #+#    #+#             */
/*   Updated: 2026/04/11 14:10:33 by gkiala           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <set>
#include <string>

#define MAX_BUFFER_SIZE 4096
#define MAX_LINE_LENGTH 512

class Client {
  private:
    int _fd;
    std::string _host;
    std::string _recvBuffer;
    std::string _sendBuffer;

    bool _passAccepted;
    bool _registered;
    bool _shouldDisconnect;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _awayMessage;

    std::set<std::string> _joinedChannels;

  public:
    Client();
    Client(int fd, const std::string &host);

    int getFd() const;
    const std::string &getHost() const;
    const std::string &getNickname() const;
    const std::string &getUsername() const;
    const std::string &getRealname() const;
    const std::string &getRecvBuffer() const;
    const std::string &getSendBuffer() const;
    const std::string &getAwayMessage() const;
    const std::set<std::string> &getJoinedChannels() const;

    bool isPassAccepted() const;
    bool isRegistered() const;
    bool isAway() const;
    bool hasNick() const;
    bool hasUser() const;

    void setPassAccepted(bool status);
    void setRegistered(bool status);
    void setNickname(const std::string &nickname);
    void setUsername(const std::string &username);
    void setRealname(const std::string &realname);
    void setAwayMessage(const std::string &message);

    void appendRecvData(const std::string &chunk);
    bool popNextLine(std::string &line);

    void queueSendData(const std::string &data);
    bool hasPendingSend() const;
    void consumeSentBytes(size_t count);

    void joinChannel(const std::string &channelName);
    void leaveChannel(const std::string &channelName);
    bool isInChannel(const std::string &channelName) const;

    bool shouldDisconnect() const;
    void markForDisconnection();
};

#endif