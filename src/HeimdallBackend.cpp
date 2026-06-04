#include "HeimdallBackend.hpp"
#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>

using namespace v1::Heimdall;

HeimdallBackend::HeimdallBackend(QObject *parent)
    : QObject(parent), m_versionNumber("unknown"), m_updateStatus("checking..."), m_running(true)
{
    m_proxyThread = std::thread(&HeimdallBackend::initProxy, this);
}

HeimdallBackend::~HeimdallBackend()
{
    m_running = false;
    if (m_proxyThread.joinable()) {
        m_proxyThread.join();
    }
}

QString HeimdallBackend::versionNumber() const
{
    return m_versionNumber;
}

QString HeimdallBackend::updateStatus() const
{
    return m_updateStatus;
}

void HeimdallBackend::initProxy()
{
    CommonAPI::Runtime::setProperty("LibraryBase", "Heimdall-HomeUI");
    auto runtime = CommonAPI::Runtime::get();

    std::string domain = "local";
    std::string instance = "Heimdall.OTASystemManagement";
    
    m_proxy = runtime->buildProxy<OTASystemManagementProxy>(domain, instance);

    std::cout << "[Backend] Checking service availability..." << std::endl;
    while (!m_proxy->isAvailable() && m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!m_running) return;

    std::cout << "[Backend] Service Available!" << std::endl;

    QMetaObject::invokeMethod(this, [this]() {
        m_updateStatus = "up-to-date";
        emit updateStatusChanged();
    }, Qt::QueuedConnection);

    m_proxy->getNewUpdateAvailableEvent().subscribe([this](const std::string& version, const std::string& url, const std::string& checksum) {
        this->onBroadcastReceived(version, url, checksum);
    });
    
    m_proxy->getLatestVersionAttribute().getChangedEvent().subscribe([this](const std::string& val) {
        QMetaObject::invokeMethod(this, [this, val]() {
            m_versionNumber = QString::fromStdString(val);
            emit versionChanged();
        }, Qt::QueuedConnection);
    });
    
    m_proxy->getCurrentStateAttribute().getChangedEvent().subscribe([this](const OTASystemManagement::UpdateState& state) {
        QMetaObject::invokeMethod(this, [this, state]() {
            if (state == OTASystemManagement::UpdateState::UPDATE_AVAILABLE) {
                m_updateStatus = "Update Available";
            } else if (state == OTASystemManagement::UpdateState::DOWNLOADING) {
                m_updateStatus = "Downloading...";
            }
            emit updateStatusChanged();
        }, Qt::QueuedConnection);
    });

    // Do initial query
    CommonAPI::CallStatus callStatus;
    std::string v, i, c;
    m_proxy->CheckForUpdates("RPi3", callStatus, v, i, c);
    if (callStatus == CommonAPI::CallStatus::SUCCESS) {
        QMetaObject::invokeMethod(this, [this, v]() {
            m_versionNumber = QString::fromStdString(v);
            emit versionChanged();
        }, Qt::QueuedConnection);
    }
}

void HeimdallBackend::onBroadcastReceived(const std::string& version, const std::string& url, const std::string& checksum)
{
    std::cout << "[Backend] Broadcast received: new version " << version << std::endl;
    
    QMetaObject::invokeMethod(this, [this, version]() {
        m_versionNumber = QString::fromStdString(version);
        m_updateStatus = "Update Available";
        emit versionChanged();
        emit updateStatusChanged();
    }, Qt::QueuedConnection);

    // Launch flash script in background
    std::string cmd = "/usr/bin/flash_update.sh " + version + " &";
    system(cmd.c_str());
}

void HeimdallBackend::checkForUpdates()
{
    if (m_proxy && m_proxy->isAvailable()) {
        CommonAPI::CallStatus callStatus;
        std::string v, i, c;
        m_proxy->CheckForUpdates("RPi3", callStatus, v, i, c);
        if (callStatus == CommonAPI::CallStatus::SUCCESS) {
            m_versionNumber = QString::fromStdString(v);
            emit versionChanged();
        }
    }
}
