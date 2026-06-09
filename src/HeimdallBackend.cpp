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

    // Subscribe to new update broadcasts
    m_proxy->getNewUpdateAvailableEvent().subscribe([this](const std::string& v, const std::string& u, const std::string& c) {
        this->onBroadcastReceived(v, u, c);
    });

    // Subscribe to state changes
    m_proxy->getCurrentStateAttribute().getChangedEvent().subscribe([this](const OTASystemManagement::UpdateState& state) {
        QMetaObject::invokeMethod(this, [this, state]() {
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
        }, Qt::QueuedConnection);
    });

    // Do initial query
    CommonAPI::CallStatus callStatus;
    std::string v, i, c;
    m_proxy->CheckForUpdates("RPi3", callStatus, v, i, c);
    if (callStatus == CommonAPI::CallStatus::SUCCESS) {
        QMetaObject::invokeMethod(this, [this, v]() {
            if (!v.empty()) {
                m_versionNumber = QString::fromStdString(v);
                emit versionChanged();
            }
        }, Qt::QueuedConnection);
    }
}

void HeimdallBackend::onBroadcastReceived(const std::string& version, const std::string& url, const std::string& checksum)
{
    std::cout << "[Backend] Broadcast received: new version " << version << std::endl;
    
    QMetaObject::invokeMethod(this, [this, version]() {
        if (m_versionNumber.toStdString() != version) {
            m_versionNumber = QString::fromStdString(version);
            emit versionChanged();
        }
        m_updateStatus = "Update Available";
        emit updateStatusChanged();
        emit updateCheckFinished(true, true, QString::fromStdString(version));
    }, Qt::QueuedConnection);
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
        CommonAPI::CallStatus cs;
        std::string v, url;
        m_proxy->getLatestVersionAttribute().getValue(cs, v);
        m_proxy->getImageURLAttribute().getValue(cs, url);
        
        std::string filename = url.substr(url.find_last_of('/') + 1);
        if (filename.empty()) filename = "rootfs.ext3";

        std::string cmd = "/usr/bin/flash_update.sh " + v + " " + filename + " &";
        system(cmd.c_str());
        m_updateStatus = "Applying Update...";
        emit updateStatusChanged();
    }
}
