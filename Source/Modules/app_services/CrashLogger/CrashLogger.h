#pragma once
#include <juce_core/juce_core.h>
#include <csignal>
#include <fstream>

namespace app_services {

/**
 * Advanced crash and resource monitoring logger
 * - Detects crashes with signal handlers
 * - Monitors CPU and RAM usage
 * - Logs only errors and significant resource spikes
 * - Generates detailed crash reports
 */
class CrashLogger {
public:
    CrashLogger(const juce::String& appName);
    ~CrashLogger();

    // Enable/disable crash detection
    void enableCrashDetection(bool enable);

    // Enable/disable resource monitoring
    void enableResourceMonitoring(bool enable, int intervalMs = 1000);

    // Log an error
    static void logError(const juce::String& component, const juce::String& message);

    // Log a warning
    static void logWarning(const juce::String& component, const juce::String& message);

    // Check current resource usage
    struct ResourceUsage {
        double cpuPercent = 0.0;
        size_t ramMB = 0;
        size_t ramPercentage = 0;
        juce::int64 timestamp = 0;
    };

    static ResourceUsage getCurrentResourceUsage();

    // Set thresholds for resource alerts
    void setResourceThresholds(double cpuThreshold = 80.0, size_t ramThresholdMB = 500);

private:
    juce::String applicationName;
    juce::File logDirectory;
    juce::File errorLogFile;
    juce::File crashReportFile;
    juce::File resourceLogFile;

    bool crashDetectionEnabled = false;
    bool resourceMonitoringEnabled = false;

    double cpuThreshold = 80.0;
    size_t ramThresholdMB = 500;

    // Resource monitoring
    class ResourceMonitor : public juce::Timer {
    public:
        ResourceMonitor(CrashLogger* logger);
        void timerCallback() override;

    private:
        CrashLogger* owner;
        ResourceUsage lastUsage;
        size_t spikeCount = 0;
    };

    std::unique_ptr<ResourceMonitor> resourceMonitor;

    // Signal handlers
    static CrashLogger* instance;
    static void signalHandler(int signal);

    // Helper methods
    void writeCrashReport(int signal, const juce::String& signalName);
    void writeResourceSpike(const ResourceUsage& usage, const juce::String& reason);
    juce::String getStackTrace();
    juce::String getSystemInfo();

    static juce::String formatTimestamp();
    static void writeToFile(const juce::File& file, const juce::String& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrashLogger)
};

} // namespace app_services
