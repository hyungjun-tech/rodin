#pragma once

class MessageError : public QObject { Q_OBJECT };
class MessageAlert : public QObject { Q_OBJECT };
class MessageInfo : public QObject { Q_OBJECT };
class MessageQuestion : public QObject { Q_OBJECT };
class MessageProgress : public QObject { Q_OBJECT };
class CustomTranslate : public QObject { Q_OBJECT };