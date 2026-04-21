/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gkiala <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 14:13:22 by gkiala            #+#    #+#             */
/*   Updated: 2026/04/11 14:13:25 by gkiala           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel() : _inviteOnly(false), _topicRestricted(false), _userLimit(0) {}

Channel::Channel(const std::string &name)
    : _name(name), _inviteOnly(false), _topicRestricted(false), _userLimit(0) {}

const std::string &Channel::getName() const { return _name; }

const std::string &Channel::getTopic() const { return _topic; }

const std::string &Channel::getKey() const { return _key; }

bool Channel::isInviteOnly() const { return _inviteOnly; }

bool Channel::isTopicRestricted() const { return _topicRestricted; }

int Channel::getUserLimit() const { return _userLimit; }

const std::set<int> &Channel::getMembers() const { return _members; }

const std::set<int> &Channel::getOperators() const { return _operators; }

bool Channel::isMember(int fd) const { return _members.find(fd) != _members.end(); }

bool Channel::isOperator(int fd) const { return _operators.find(fd) != _operators.end(); }

bool Channel::isInvited(int fd) const { return _invited.find(fd) != _invited.end(); }

bool Channel::isFull() const {
  return _userLimit > 0 && static_cast<int>(_members.size()) >= _userLimit;
}

void Channel::setTopic(const std::string &topic) { _topic = topic; }

void Channel::setKey(const std::string &key) { _key = key; }

void Channel::setInviteOnly(bool state) { _inviteOnly = state; }

void Channel::setTopicRestricted(bool state) { _topicRestricted = state; }

void Channel::setUserLimit(int limit) { _userLimit = limit; }

void Channel::addMember(int fd) { _members.insert(fd); }

void Channel::removeMember(int fd) {
  _members.erase(fd);
  _operators.erase(fd);
  _invited.erase(fd);
}

void Channel::addOperator(int fd) { _operators.insert(fd); }

void Channel::removeOperator(int fd) { _operators.erase(fd); }

void Channel::invite(int fd) { _invited.insert(fd); }

void Channel::uninvite(int fd) { _invited.erase(fd); }

bool Channel::empty() const { return _members.empty(); }

