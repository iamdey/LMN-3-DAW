#include "CrashLogger.h"
#include <sys/resource.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>

namespace app_services {

CrashLogger* CrashLogger::instance = nullptr;

CrashLogger::CrashLogger(const juce::String& appName)
    : applicationName(appName) {

    // Setup log directory (create recursively if needed)
    auto userAppData = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    logDirectory = userAppData.getChildFile(appName).getChildFile("logs");

    if (!logDirectory.exists()) {
        juce::Result result = logDirectory.createDirectory();
        if (result.failed()) {
            // Fallback: try creating parent directory first
            auto parentDir = userAppData.getChildFile(appName);
            parentDir.createDirectory();
            logDirectory.createDirectory();
        }
    }

    // Setup log files
    errorLogFile = logDirectory.getChildFile("errors.log");
    crashReportFile = logDirectory.getChildFile("crash_report.log");
    resourceLogFile = logDirectory.getChildFile("resource_spikes.log");

    // Clean old logs (keep only last 5 files)
    auto logFiles = logDirectory.findChildFiles(juce::File::findFiles, false, "*.log");
    if (logFiles.size() > 15) {
        logFiles.sort();
        for (int i = 0; i < logFiles.size() - 15; i++) {
            logFiles[i].deleteFile();
        }
    }

    instance = this;
}

CrashLogger::~CrashLogger() {
    enableCrashDetection(false);
    enableResourceMonitoring(false);
    instance = nullptr;
}

void CrashLogger::enableCrashDetection(bool enable) {
    if (enable && !crashDetectionEnabled) {
        // Register signal handlers for common crashes
        std::signal(SIGSEGV, signalHandler);  // Segmentation fault
        std::signal(SIGABRT, signalHandler);  // Abort
        std::signal(SIGFPE, signalHandler);   // Floating point exception
        std::signal(SIGILL, signalHandler);   // Illegal instruction
        crashDetectionEnabled = true;

        logError("CrashLogger", "Crash detection enabled");
    } else if (!enable && crashDetectionEnabled) {
        std::signal(SIGSEGV, SIG_DFL);
        std::signal(SIGABRT, SIG_DFL);
        std::signal(SIGFPE, SIG_DFL);
        std::signal(SIGILL, SIG_DFL);
        crashDetectionEnabled = false;
    }
}

void CrashLogger::enableResourceMonitoring(bool enable, int intervalMs) {
    if (enable && !resourceMonitoringEnabled) {
        resourceMonitor = std::make_unique<ResourceMonitor>(this);
        resourceMonitor->startTimer(intervalMs);
        resourceMonitoringEnabled = true;

        logError("CrashLogger", "Resource monitoring enabled (interval: " + juce::String(intervalMs) + "ms)");
    } else if (!enable && resourceMonitoringEnabled) {
        resourceMonitor = nullptr;
        resourceMonitoringEnabled = false;
    }
}

void CrashLogger::setResourceThresholds(double cpuThresh, size_t ramThreshMB) {
    cpuThreshold = cpuThresh;
    ramThresholdMB = ramThreshMB;
}

void CrashLogger::logError(const juce::String& component, const juce::String& message) {
    if (!instance) return;

    juce::String logEntry = formatTimestamp() + " [ERROR] [" + component + "] " + message + "\n";

    // Write directly (thread-safe, no MessageManager dependency)
    writeToFile(instance->errorLogFile, logEntry);
}

void CrashLogger::logWarning(const juce::String& component, const juce::String& message) {
    if (!instance) return;

    juce::String logEntry = formatTimestamp() + " [WARN] [" + component + "] " + message + "\n";

    // Write directly (thread-safe, no MessageManager dependency)
    writeToFile(instance->errorLogFile, logEntry);
}

CrashLogger::ResourceUsage CrashLogger::getCurrentResourceUsage() {
    ResourceUsage usage;
    usage.timestamp = juce::Time::currentTimeMillis();

    // Get memory usage (RSS - Resident Set Size)
    struct rusage rusage;
    if (getrusage(RUSAGE_SELF, &rusage) == 0) {
        // rusage.ru_maxrss is in KB on Linux
        usage.ramMB = rusage.ru_maxrss / 1024;
    }

    // Get CPU usage by reading /proc/self/stat (Linux specific, lightweight)
    static long lastTotalTime = 0;
    static long lastProcessTime = 0;
    static auto lastCheckTime = std::chrono::steady_clock::now();

    std::ifstream stat("/proc/self/stat");
    if (stat.is_open()) {
        std::string dummy;
        long utime, stime;

        // Skip first 13 fields
        for (int i = 0; i < 13; i++) stat >> dummy;
        stat >> utime >> stime;

        long processTime = utime + stime;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCheckTime).count();

        if (lastProcessTime > 0 && elapsed > 0) {
            long processDelta = processTime - lastProcessTime;
            // Clock ticks to milliseconds (assuming 100Hz clock)
            double processTimeMs = processDelta * 10.0;
            usage.cpuPercent = (processTimeMs / elapsed) * 100.0;
        }

        lastProcessTime = processTime;
        lastCheckTime = now;
    }

    // Calculate RAM percentage (assume 1GB total for Raspberry Pi)
    usage.ramPercentage = (usage.ramMB * 100) / 1024;

    return usage;
}

void CrashLogger::signalHandler(int signal) {
    if (!instance) return;

    juce::String signalName;
    switch (signal) {
        case SIGSEGV: signalName = "SIGSEGV (Segmentation Fault)"; break;
        case SIGABRT: signalName = "SIGABRT (Abort)"; break;
        case SIGFPE:  signalName = "SIGFPE (Floating Point Exception)"; break;
        case SIGILL:  signalName = "SIGILL (Illegal Instruction)"; break;
        default:      signalName = "Unknown Signal (" + juce::String(signal) + ")"; break;
    }

    instance->writeCrashReport(signal, signalName);

    // Re-raise signal to let system handle it
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}

void CrashLogger::writeCrashReport(int signal, const juce::String& signalName) {
    std::stringstream report;

    report << "================================\n";
    report << "CRASH REPORT\n";
    report << "================================\n";
    report << "Timestamp: " << formatTimestamp().toStdString() << "\n";
    report << "Application: " << applicationName.toStdString() << "\n";
    report << "Signal: " << signalName.toStdString() << "\n";
    report << "\n";

    report << "System Information:\n";
    report << getSystemInfo().toStdString() << "\n";
    report << "\n";

    report << "Resource Usage at Crash:\n";
    auto usage = getCurrentResourceUsage();
    report << "  CPU: " << std::fixed << std::setprecision(1) << usage.cpuPercent << "%\n";
    report << "  RAM: " << usage.ramMB << " MB (" << usage.ramPercentage << "%)\n";
    report << "\n";

    report << "Stack Trace:\n";
    report << getStackTrace().toStdString() << "\n";
    report << "================================\n";

    // Write directly (crash situation, no async)
    std::ofstream file(crashReportFile.getFullPathName().toStdString(), std::ios::app);
    if (file.is_open()) {
        file << report.str();
        file.close();
    }
}

void CrashLogger::writeResourceSpike(const ResourceUsage& usage, const juce::String& reason) {
    std::stringstream report;

    report << formatTimestamp().toStdString() << " ";
    report << "[RESOURCE SPIKE] " << reason.toStdString() << " - ";
    report << "CPU: " << std::fixed << std::setprecision(1) << usage.cpuPercent << "%, ";
    report << "RAM: " << usage.ramMB << "MB (" << usage.ramPercentage << "%)\n";

    writeToFile(resourceLogFile, juce::String(report.str()));
}

juce::String CrashLogger::getStackTrace() {
    // Basic stack trace (can be enhanced with backtrace() on Linux)
    return "Stack trace not available (lightweight mode)";
}

juce::String CrashLogger::getSystemInfo() {
    std::stringstream info;

    // OS info
    struct utsname sysinfo;
    if (uname(&sysinfo) == 0) {
        info << "  OS: " << sysinfo.sysname << " " << sysinfo.release << "\n";
        info << "  Machine: " << sysinfo.machine << "\n";
    }

    // Process info
    info << "  PID: " << getpid() << "\n";

    return juce::String(info.str());
}

juce::String CrashLogger::formatTimestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return juce::String(ss.str());
}

void CrashLogger::writeToFile(const juce::File& file, const juce::String& message) {
    std::ofstream stream(file.getFullPathName().toStdString(), std::ios::app);
    if (stream.is_open()) {
        stream << message.toStdString();
        stream.close();
    }
}

// ResourceMonitor implementation
CrashLogger::ResourceMonitor::ResourceMonitor(CrashLogger* logger)
    : owner(logger), spikeCount(0) {
    lastUsage = getCurrentResourceUsage();
}

void CrashLogger::ResourceMonitor::timerCallback() {
    auto currentUsage = getCurrentResourceUsage();

    bool cpuSpike = currentUsage.cpuPercent > owner->cpuThreshold;
    bool ramSpike = currentUsage.ramMB > owner->ramThresholdMB;

    if (cpuSpike || ramSpike) {
        spikeCount++;

        juce::String reason;
        if (cpuSpike && ramSpike) {
            reason = "CPU and RAM spike detected";
        } else if (cpuSpike) {
            reason = "CPU spike detected";
        } else {
            reason = "RAM spike detected";
        }

        // Only log every 5th spike to avoid flooding
        if (spikeCount % 5 == 1) {
            owner->writeResourceSpike(currentUsage, reason);
        }
    } else {
        spikeCount = 0; // Reset counter when back to normal
    }

    lastUsage = currentUsage;
}

} // namespace app_services
