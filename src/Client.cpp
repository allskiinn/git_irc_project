/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkiala <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 14:13:11 by gkiala            #+#    #+#             */
/*   Updated: 2026/04/11 14:13:13 by gkiala           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <cstddef>

Client::Client() 
    : _fd(-1), _passAccepted(false), _registered(false), _shouldDisconnect(false) {}

Client::Client(int fd, const std::string &host)
    : _fd(fd), _host(host), _passAccepted(false), _registered(false), _shouldDisconnect(false) {}

int Client::getFd() const { return _fd; }

const std::string &Client::getHost() const { return _host; }

const std::string &Client::getNickname() const { return _nickname; }

const std::string &Client::getUsername() const { return _username; }

const std::string &Client::getRealname() const { return _realname; }

const std::string &Client::getRecvBuffer() const { return _recvBuffer; }

const std::string &Client::getSendBuffer() const { return _sendBuffer; }

const std::string &Client::getAwayMessage() const { return _awayMessage; }

const std::set<std::string> &Client::getJoinedChannels() const { return _joinedChannels; }

bool Client::isPassAccepted() const { return _passAccepted; }

bool Client::isRegistered() const { return _registered; }

bool Client::isAway() const { return !_awayMessage.empty(); }

bool Client::hasNick() const { return !_nickname.empty(); }

bool Client::hasUser() const { return !_username.empty(); }

bool Client::shouldDisconnect() const { return _shouldDisconnect; }

void Client::setPassAccepted(bool status) { _passAccepted = status; }

void Client::setRegistered(bool status) { _registered = status; }

void Client::setNickname(const std::string &nickname) { _nickname = nickname; }

void Client::setUsername(const std::string &username) { _username = username; }

void Client::setRealname(const std::string &realname) { _realname = realname; }

void Client::setAwayMessage(const std::string &message) { _awayMessage = message; }

void Client::markForDisconnection() { _shouldDisconnect = true; }

void Client::appendRecvData(const std::string &chunk)
{
    _recvBuffer += chunk;

    if (_recvBuffer.size() > MAX_BUFFER_SIZE)
    {
        _shouldDisconnect = true;
    }
}

bool Client::popNextLine(std::string &line)
{
    std::string::size_type pos = _recvBuffer.find('\n');

    if (pos == std::string::npos) {
        return false;
    }

    if (pos > MAX_LINE_LENGTH) {
        _recvBuffer.erase(0, pos + 1);
        return false;
    }

    line = _recvBuffer.substr(0, pos);

    if (!line.empty() && line[line.size() - 1] == '\r') {
        line.erase(line.size() - 1);
    }

    _recvBuffer.erase(0, pos + 1);
    return true;
}

void Client::queueSendData(const std::string &data) {
    if (data.empty()) {
        return;
    }
    _sendBuffer += data;
}

bool Client::hasPendingSend() const {
    return !_sendBuffer.empty();
}

void Client::consumeSentBytes(size_t count) {
    if (count == 0) {
        return;
    }
    if (count >= _sendBuffer.size()) {
        _sendBuffer.clear();
        return;
    }
    _sendBuffer.erase(0, count);
}

void Client::joinChannel(const std::string &channelName) {
    _joinedChannels.insert(channelName);
}

void Client::leaveChannel(const std::string &channelName) {
    _joinedChannels.erase(channelName);
}

bool Client::isInChannel(const std::string &channelName) const {
    return _joinedChannels.find(channelName) != _joinedChannels.end();
}