#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

class QListWidget;
class QListWidgetItem;
class QLabel;
class QPushButton;
class ChatWindow;
class NetworkClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &currentUser, NetworkClient *networkClient, QWidget *parent = nullptr);

private slots:
    void onRefreshClicked();
    void onUserListReceived(const QStringList &users);
    void onUserDoubleClicked(QListWidgetItem *item);
    void onChatMessageReceived(const QString &from, const QString &to, const QString &message, const QString &time);

private:
    ChatWindow *openChatWindow(const QString &peerUser);

    QString m_currentUser;
    NetworkClient *m_networkClient;
    QLabel *m_statusLabel;
    QListWidget *m_userListWidget;
    QPushButton *m_refreshButton;
    QMap<QString, ChatWindow *> m_chatWindows;
};

#endif
