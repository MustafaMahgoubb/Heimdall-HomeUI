#include "HeimdallBackend.hpp"
#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <fstream>

using namespace v1::Heimdall;

HeimdallBackend::HeimdallBackend(QObject *parent)
    : QObject(parent), m_updateStatus("checking..."), m_running(true)
{
    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &HeimdallBackend::pollServer);
    m_pollTimer->start(3000);
    std::ifstream vfile("/etc/heimdall_version.txt");
    std::string ver;
    if (vfile.is_open() && std::getline(vfile, ver)) {
        m_versionNumber = QString::fromStdString(ver);
        vfile.close();
    } else {
        m_versionNumber = "v1.0.0";
    }

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

    if (!m_proxy) {
        std::cerr << "[Backend] Error: Failed to build CommonAPI proxy! (Check if libHeimdall-HomeUI-someip.so is loaded)" << std::endl;
        QMetaObject::invokeMethod(this, [this]() {
            m_updateStatus = "Error: Proxy Build Failed";
            emit updateStatusChanged();
        }, Qt::QueuedConnection);
        return;
    }

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

    // Event subscriptions removed due to Service Discovery hardware constraints
    // UI state is now polled via pollServer() QTimer

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
    
    m_updateStatus = "Update Available";
    emit updateStatusChanged();
    emit updateCheckFinished(true, true, QString::fromStdString(version));
}

void HeimdallBackend::pollServer()
{
    if (m_proxy && m_proxy->isAvailable()) {
        CommonAPI::CallStatus cs;
        std::string val;
        m_proxy->getLatestVersionAttribute().getValue(cs, val);
        if (cs == CommonAPI::CallStatus::SUCCESS) {
            QString newVal = QString::fromStdString(val);
            if (m_versionNumber != newVal) {
                m_versionNumber = newVal;
                emit versionChanged();
            }
        }
        
        CommonAPI::CallStatus csState;
        OTASystemManagement::UpdateState state;
        m_proxy->getCurrentStateAttribute().getValue(csState, state);
        if (csState == CommonAPI::CallStatus::SUCCESS) {
            if (state == OTASystemManagement::UpdateState::UPDATE_AVAILABLE) {
                if (m_updateStatus == "Downloading...") {
                    m_updateStatus = "Download Complete";
                    emit updateStatusChanged();
                    emit downloadComplete();
                } else if (m_updateStatus != "Update Available" && m_updateStatus != "Download Complete") {
                    m_updateStatus = "Update Available";
                    emit updateStatusChanged();
                }
            } else if (state == OTASystemManagement::UpdateState::DOWNLOADING) {
                if (m_updateStatus != "Downloading...") {
                    m_updateStatus = "Downloading...";
                    emit updateStatusChanged();
                }
            } else if (state == OTASystemManagement::UpdateState::ERROR) {
                if (m_updateStatus != "Error") {
                    m_updateStatus = "Error";
                    emit updateStatusChanged();
                }
            }
        }
    }
}

void HeimdallBackend::checkForUpdates()
{
    if (m_proxy && m_proxy->isAvailable()) {
        CommonAPI::CallStatus callStatus;
        std::string v, i, c;
        m_proxy->CheckForUpdates("RPi3", callStatus, v, i, c);
        if (callStatus == CommonAPI::CallStatus::SUCCESS) {
            bool isNew = (!v.empty() && v != m_versionNumber.toStdString());
            emit updateCheckFinished(true, isNew, QString::fromStdString(v));
        } else {
            emit updateCheckFinished(false, false, "");
        }
    } else {
        emit updateCheckFinished(false, false, "");
    }
}

void HeimdallBackend::requestDownload()
{
    if (m_proxy && m_proxy->isAvailable()) {
        CommonAPI::CallStatus callStatus;
        bool ack;
        m_proxy->ReportState(OTASystemManagement::UpdateState::DOWNLOADING, "RPi3", callStatus, ack);
    }
}

void HeimdallBackend::applyUpdate()
{
    if (m_proxy && m_proxy->isAvailable()) {
        CommonAPI::CallStatus callStatus;
        CommonAPI::CallStatus cs;
        std::string v;
        m_proxy->getLatestVersionAttribute().getValue(cs, v);
        std::string cmd = "/usr/bin/flash_update.sh " + v + " &";
        system(cmd.c_str());
        m_updateStatus = "Applying Update...";
        emit updateStatusChanged();
    }
}
