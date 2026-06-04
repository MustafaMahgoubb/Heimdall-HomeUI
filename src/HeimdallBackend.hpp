#ifndef HEIMDALL_BACKEND_HPP
#define HEIMDALL_BACKEND_HPP

#include <QObject>
#include <QString>
#include <QThread>
#include <CommonAPI/CommonAPI.hpp>
#include <v1/Heimdall/OTASystemManagementProxy.hpp>

class HeimdallBackend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString versionNumber READ versionNumber NOTIFY versionChanged)
    Q_PROPERTY(QString updateStatus READ updateStatus NOTIFY updateStatusChanged)

public:
    explicit HeimdallBackend(QObject *parent = nullptr);
    ~HeimdallBackend();

    QString versionNumber() const;
    QString updateStatus() const;

    Q_INVOKABLE void checkForUpdates();

signals:
    void versionChanged();
    void updateStatusChanged();
    void newUpdateAvailable(const QString& version, const QString& url, const QString& checksum);
    void updateCheckFinished(bool success, bool isNewUpdate, const QString& latestVersion);

private:
    void initProxy();
    void onBroadcastReceived(const std::string& version, const std::string& url, const std::string& checksum);

    QString m_versionNumber;
    QString m_updateStatus;
    std::shared_ptr<v1::Heimdall::OTASystemManagementProxy<>> m_proxy;
    std::thread m_proxyThread;
    bool m_running;
};

#endif
