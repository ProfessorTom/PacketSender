//
// Created by Tomas Gallucci on 2/28/26.
//

#ifndef PERSISTENTCONNECTIONMANAGER_H
#define PERSISTENTCONNECTIONMANAGER_H


#include <QObject>
#include <QList>

class PersistentConnection;
class TCPThread;
class PacketNetwork;

    #ifndef CONSOLE_BUILD
    class PersistentConnectionManager : public QObject
    {
        Q_OBJECT

    public:
        explicit PersistentConnectionManager(PacketNetwork *parent = nullptr);
        ~PersistentConnectionManager();

        void registerConnection(PersistentConnection *window, TCPThread *thread);
        void unregisterConnection(TCPThread *thread);  // called on finished()

        void shutdownAll();

    private:
        QList<PersistentConnection*> m_windows;
        QList<TCPThread*>            m_threads;
        PacketNetwork               *m_network;  // non-owning back-pointer

    private slots:
        void onThreadFinished();
    };

    #endif  // CONSOLE_BUILD
#endif //PERSISTENTCONNECTIONMANAGER_H
