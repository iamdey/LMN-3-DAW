#include "ConfigurationHelpers.h"
#include <yaml-cpp/yaml.h>

juce::File ConfigurationHelpers::SAVED_TRACK_NAME;

void ConfigurationHelpers::setSavedTrackName(const juce::File &newValue) {
    SAVED_TRACK_NAME = newValue;
}

juce::File ConfigurationHelpers::getSavedTrackName() {
    return SAVED_TRACK_NAME;
}

juce::String ConfigurationHelpers::getApplicationName() {
    return ROOT_DIRECTORY_NAME;
}

bool ConfigurationHelpers::writeBinarySamplesToDirectory(
    const juce::File &destDir, juce::StringRef filename, const char *data,
    int dataSizeInBytes) {
    auto f = destDir.getChildFile(filename);
    jassert(data != nullptr);
    return f.replaceWithData(data, static_cast<size_t>(dataSizeInBytes));
}

void ConfigurationHelpers::initBinarySamples(
    const juce::File & /*tempSynthDir*/, const juce::File & /*tempDrumDir*/) {
    //    NB: Binary samples are currently not used, so the sample data
    //    libraries do not get built which is why this is commented out for (int
    //    i = 0; i < SynthSampleData::namedResourceListSize; ++i) {
    //        int dataSizeInBytes = 0;
    //        const char *data = SynthSampleData::getNamedResource(
    //            SynthSampleData::namedResourceList[i], dataSizeInBytes);
    //        auto success = writeBinarySampleToDirectory(
    //            tempSynthDir, SynthSampleData::originalFilenames[i], data,
    //            dataSizeInBytes);
    //        if (!success) {
    //            juce::Logger::writeToLog(
    //                "Attempt to write binary synth sample data file
    //                failed!");
    //        }
    //    }
    //
    //    for (int i = 0; i < DrumSampleData::namedResourceListSize; ++i) {
    //        int dataSizeInBytes = 0;
    //        const char *data = DrumSampleData::getNamedResource(
    //            DrumSampleData::namedResourceList[i], dataSizeInBytes);
    //        auto success = writeBinarySampleToDirectory(
    //            tempDrumDir, DrumSampleData::originalFilenames[i], data,
    //            dataSizeInBytes);
    //        if (!success) {
    //            juce::Logger::writeToLog(
    //                "Attempt to write binary drum sample data file
    //                failed!");
    //        }
    //    }
}

void syncUserFilesIfNeeded(const juce::File &sourceDir,
                           const juce::File &targetDir,
                           const juce::String &label) {
    if (!sourceDir.exists()) {
        juce::Logger::writeToLog("User " + label +
                                 " directory does not exist, creating it now.");
        auto result = sourceDir.createDirectory();

        if (result.failed()) {
            juce::Logger::writeToLog("Failed to create user " + label +
                                     " directory: " + result.getErrorMessage());
        }

        return;
    }

    bool allSuccessful = true;
    int filesCopied = 0;

    for (const auto &srcFile :
         sourceDir.findChildFiles(juce::File::findFiles, true)) {
        auto relativePath = srcFile.getRelativePathFrom(sourceDir);
        juce::File destFile = targetDir.getChildFile(relativePath);

        if (!destFile.existsAsFile() ||
            srcFile.getLastModificationTime() >
                destFile.getLastModificationTime()) {
            destFile.getParentDirectory().createDirectory();

            if (!srcFile.copyFileTo(destFile)) {
                juce::Logger::writeToLog("Failed to copy: " +
                                         srcFile.getFullPathName());
                allSuccessful = false;
            } else {
                ++filesCopied;
            }
        }
    }

    if (allSuccessful) {
        juce::Logger::writeToLog("User " + label +
                                 " files copied (modified only). Total: " +
                                 juce::String(filesCopied));
    } else {
        juce::Logger::writeToLog("Some user " + label +
                                 " files failed to copy.");
    }
}

void ConfigurationHelpers::initUserSamples(const juce::File &userSynthSampleDir,
                                           const juce::File &userDrumDir,
                                           const juce::File &tempSynthDir,
                                           const juce::File &tempDrumDir) {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);

    syncUserFilesIfNeeded(userSynthSampleDir, tempSynthDir, "synth sample");
    syncUserFilesIfNeeded(userDrumDir, tempDrumDir, "drum kit");
}
juce::File
ConfigurationHelpers::createTempDirectory(tracktion::Engine &engine,
                                          const juce::String &folderName) {
    auto dir = engine.getTemporaryFileManager().getTempFile(folderName);
    auto result = dir.createDirectory();
    if (!result.wasOk()) {
        juce::Logger::writeToLog("Error creating temporary directory " +
                                 folderName);
        return {};
    } else {
        return dir;
    }
}

void ConfigurationHelpers::initSamples(tracktion::Engine &engine) {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    auto userSynthSampleDir = getSamplesDirectory();
    auto userDrumDir = getDrumKitsDirectory();
    auto tempSynthSamplesDir =
        createTempDirectory(engine, SAMPLES_DIRECTORY_NAME);
    auto tempDrumKitsDir =
        createTempDirectory(engine, DRUM_KITS_DIRECTORY_NAME);
    //        initBinarySamples(tempSynthSamplesDir, tempDrumKitsDir);
    initUserSamples(userSynthSampleDir, userDrumDir, tempSynthSamplesDir,
                    tempDrumKitsDir);
}

bool ConfigurationHelpers::getShowTitleBar(juce::File &configFile) {
    if (configFile.exists()) {
        YAML::Node rootNode =
            YAML::LoadFile(configFile.getFullPathName().toStdString());
        YAML::Node config = rootNode["config"];
        if (config)
            if (config["show-title-bar"])
                return config["show-title-bar"].as<bool>();
    }

    // Default to showing the title bar
    return true;
}

double ConfigurationHelpers::getWidth(juce::File &configFile) {
    if (configFile.exists()) {
        YAML::Node rootNode =
            YAML::LoadFile(configFile.getFullPathName().toStdString());
        YAML::Node config = rootNode["config"];
        if (config) {
            if (config["size"]) {
                auto sizeConfig = config["size"];
                if (sizeConfig["width"])
                    return sizeConfig["width"].as<double>();
            }
        }
    }

    // Default to 800
    return 800;
}

double ConfigurationHelpers::getHeight(juce::File &configFile) {
    if (configFile.exists()) {
        YAML::Node rootNode =
            YAML::LoadFile(configFile.getFullPathName().toStdString());
        YAML::Node config = rootNode["config"];
        if (config) {
            if (config["size"]) {
                auto sizeConfig = config["size"];
                if (sizeConfig["height"])
                    return sizeConfig["height"].as<double>();
            }
        }
    }

    // Default to 480
    return 480;
}

juce::File ConfigurationHelpers::getSamplesDirectory() {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    return userAppDataDirectory.getChildFile(ROOT_DIRECTORY_NAME)
        .getChildFile(SAMPLES_DIRECTORY_NAME);
}

juce::File ConfigurationHelpers::getDrumKitsDirectory() {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    return userAppDataDirectory.getChildFile(ROOT_DIRECTORY_NAME)
        .getChildFile(DRUM_KITS_DIRECTORY_NAME);
}

juce::File
ConfigurationHelpers::getTempSamplesDirectory(tracktion::Engine &engine) {
    return engine.getTemporaryFileManager().getTempFile(SAMPLES_DIRECTORY_NAME);
}

juce::File
ConfigurationHelpers::getTempDrumKitsDirectory(tracktion::Engine &engine) {
    return engine.getTemporaryFileManager().getTempFile(
        DRUM_KITS_DIRECTORY_NAME);
}
juce::String ConfigurationHelpers::getSavedOutputDevice() {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    auto configFile = userAppDataDirectory.getChildFile(ROOT_DIRECTORY_NAME)
                          .getChildFile("config.yaml");
    if (!configFile.existsAsFile())
        return {};

    try {
        YAML::Node rootNode =
            YAML::LoadFile(configFile.getFullPathName().toStdString());
        if (rootNode) {
            auto config = rootNode["config"];
            if (config && config["output-device"])
                return config["output-device"].as<std::string>();
        }
    } catch (const std::exception &ex) {
        juce::Logger::writeToLog(
            "Failed to read saved output device from config: " +
            juce::String(ex.what()));
    }

    return {};
}

void ConfigurationHelpers::setSavedOutputDevice(
    const juce::String &deviceName) {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    auto configDir = userAppDataDirectory.getChildFile(ROOT_DIRECTORY_NAME);
    auto result = configDir.createDirectory();
    if (result.failed()) {
        juce::Logger::writeToLog("Failed to create config directory: " +
                                 result.getErrorMessage());
        return;
    }

    auto configFile = configDir.getChildFile("config.yaml");
    YAML::Node rootNode;
    if (configFile.existsAsFile()) {
        try {
            rootNode =
                YAML::LoadFile(configFile.getFullPathName().toStdString());
        } catch (const std::exception &ex) {
            juce::Logger::writeToLog(
                "Failed to parse existing config when saving output: " +
                juce::String(ex.what()));
        }
    }

    if (!rootNode["config"])
        rootNode["config"] = YAML::Node(YAML::NodeType::Map);

    rootNode["config"]["output-device"] = deviceName.toStdString();

    YAML::Emitter emitter;
    emitter << rootNode;
    configFile.replaceWithText(emitter.c_str());
}