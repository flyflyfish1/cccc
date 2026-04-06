#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>

class QTextEdit;
class QLineEdit;
class QPushButton;
class NetworkClient;

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWindow(const QString &currentUser, const QString &peerUser, NetworkClient *networkClient, QWidget *parent = nullptr);

    QString peerUser() const;
    void appendMessage(const QString &sender, const QString &message, const QString &time);

private slots:
    void onSendClicked();
    void onSendResult(bool success, const QString &message, const QString &toUser);

private:
    QString formatLine(const QString &sender, const QString &message, const QString &time) const;

    QString m_currentUser;
    QString m_peerUser;
    NetworkClient *m_networkClient;
    QTextEdit *m_chatView;
    QLineEdit *m_inputEdit;
    QPushButton *m_sendButton;
};

#endif
