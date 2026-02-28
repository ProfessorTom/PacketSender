//
// Created by Tomas Gallucci on 2/28/26.
//

#include "persistentconnectionmanager.h"

#ifndef CONSOLE_BUILD

    #include "persistentconnection.h"
    #include "tcpthread.h"
    #include "packetnetwork.h"
    #include "globals.h"  // for QDEBUG if used

    PersistentConnectionManager::PersistentConnectionManager(PacketNetwork *parent)
        : QObject(parent),
          m_network(parent)
    {
    }

    PersistentConnectionManager::~PersistentConnectionManager()
    {
        shutdownAll();
    }

    void PersistentConnectionManager::registerConnection(PersistentConnection *window, TCPThread *thread)
    {
        if (!window || !thread) return;

        m_windows.append(window);
        m_threads.append(thread);

        connect(thread, &QThread::finished, this, &PersistentConnectionManager::onThreadFinished);
        // Optional: connect other cleanup signals if needed
    }

    void PersistentConnectionManager::unregisterConnection(TCPThread *thread)
    {
        int idx = m_threads.indexOf(thread);
        if (idx >= 0) {
            m_threads.removeAt(idx);
            m_windows.removeAt(idx);  // assumes 1:1 pairing
        }
    }

    void PersistentConnectionManager::onThreadFinished()
    {
        TCPThread *thread = qobject_cast<TCPThread*>(sender());
        if (thread) {
            unregisterConnection(thread);
        }
    }

void PersistentConnectionManager::shutdownAll()
    {
        if (m_windows.isEmpty()) return;

        QDEBUG() << "Shutting down" << m_windows.size() << "persistent connections";

        QDEBUG() << "Manager shutdown called - threads:" << m_threads.size();

        // 1. Close all windows (emits closeConnection() to threads)
        for (PersistentConnection *w : std::as_const(m_windows)) {
            if (w && w->isVisible()) {
                w->close();
            }
        }

        // 2. Force abort on sockets (wakes waitForReadyRead immediately)
        for (TCPThread *t : std::as_const(m_threads)) {
            if (t) {
                t->closeConnection();  // ensure abort() is called
            }
        }

        QDEBUG() << "After abort - still running threads:" << m_threads.size();

        // 3. Wait briefly for threads to finish (this is the key addition)
        //    Use a short timeout loop instead of one long wait to keep GUI responsive
        QElapsedTimer timer;
        timer.start();
        while (!m_threads.isEmpty() && timer.elapsed() < 1000) {  // max 1 second
            QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
            QThread::msleep(50);  // small sleep to avoid busy-wait
        }

        QDEBUG() << "After processEvents - remaining threads:" << m_threads.size();

        // 4. If any threads still running, warn (but proceed — Qt will abort anyway)
        if (!m_threads.isEmpty()) {
            QDEBUG() << "Warning: Some threads did not finish in time (" << m_threads.size() << ")";
        }

        // 5. Final cleanup
        qDeleteAll(m_windows);
        m_windows.clear();
        m_threads.clear();
    }

#endif  // CONSOLE_BUILD
