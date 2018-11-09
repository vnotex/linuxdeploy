// local headers
#include "linuxdeploy/core/desktopfile.h"
#include "linuxdeploy/core/log.h"
#include "desktopfilereader.h"
#include "desktopfilewriter.h"

using namespace linuxdeploy::core;
using namespace linuxdeploy::core::log;

namespace bf = boost::filesystem;

namespace linuxdeploy {
    namespace core {
        namespace desktopfile {
            class DesktopFile::PrivateData {
                public:
                    bf::path path;
                    sections_t data;

                public:
                    PrivateData() = default;

                    void copyData(const std::shared_ptr<PrivateData>& other) {
                        path = other->path;
                        data = other->data;
                    }

            public:
                bool isEmpty() const {
                    return data.empty();
                }
            };

            DesktopFile::DesktopFile() : d(std::make_shared<PrivateData>()) {}

            DesktopFile::DesktopFile(const bf::path& path) : DesktopFile() {
                if (bf::exists(path)) {
                    // will throw exceptions in case of issues
                    read(path);
                }
            };

            DesktopFile::DesktopFile(std::istream& is) : DesktopFile() {
                // will throw exceptions in case of issues
                read(is);
            };

            // copy constructor
            DesktopFile::DesktopFile(const DesktopFile& other) : DesktopFile() {
                d->copyData(other.d);
            }

            // copy assignment constructor
            DesktopFile& DesktopFile::operator=(const DesktopFile& other) {
                if (this != &other) {
                    d->copyData(other.d);
                }

                return *this;
            }

            // move assignment operator
            DesktopFile& DesktopFile::operator=(DesktopFile&& other) noexcept {
                if (this != &other) {
                    d = other.d;
                    other.d = nullptr;
                }
                return *this;
            }

            void DesktopFile::read(const boost::filesystem::path& path) {
                setPath(path);

                // clear data before reading a new file
                clear();

                DesktopFileReader reader(path);
                d->data = reader.data();
            }

            void DesktopFile::read(std::istream& is) {
                // clear data before reading a new file
                clear();

                DesktopFileReader reader(is);
                d->data = reader.data();
            }

            boost::filesystem::path DesktopFile::path() const {
                return d->path;
            }

            void DesktopFile::setPath(const boost::filesystem::path& path) {
                d->path = path;
            }

            bool DesktopFile::isEmpty() const {
                return d->isEmpty();
            }

            void DesktopFile::clear() {
                d->data.clear();
            }

            bool DesktopFile::save() const {
                return save(d->path);
            }

            bool DesktopFile::save(const boost::filesystem::path& path) const {
                DesktopFileWriter writer(d->data);
                writer.save(path);

                return true;
            }

            bool DesktopFile::save(std::ostream& os) const {
                DesktopFileWriter writer(d->data);
                writer.save(os);

                return true;
            }

            bool DesktopFile::entryExists(const std::string& section, const std::string& key) const {
                auto it = d->data.find(section);
                if (it == d->data.end())
                    return false;

                return (it->second.find(key) != it->second.end());
            }

            bool DesktopFile::setEntry(const std::string& section, const std::string& key, const std::string& value) {
                // check if value exists -- used for return value
                auto rv = entryExists(section, key);

                d->data[section][key] = std::move(DesktopFileEntry(key, value));

                return rv;
            }

            bool DesktopFile::setEntry(const std::string& section, const DesktopFileEntry& entry) {
                // check if value exists -- used for return value
                auto rv = entryExists(section, entry.key());

                d->data[section][entry.key()] = entry;

                return rv;
            }

            bool DesktopFile::setEntry(const std::string& section, const DesktopFileEntry&& entry) {
                  // check if value exists -- used for return value
                auto rv = entryExists(section, entry.key());

                d->data[section][entry.key()] = entry;

                return rv;
            }

            bool DesktopFile::getEntry(const std::string& section, const std::string& key, std::string& value) const {
                if (!entryExists(section, key))
                    return false;

                value = d->data[section][key].value();

                return true;
            }

            bool DesktopFile::getEntry(const std::string& section, const std::string& key, DesktopFileEntry& entry) const {
                if (!entryExists(section, key))
                    return false;

                entry = d->data[section][key];
                return true;
            }

            bool DesktopFile::addDefaultKeys(const std::string& executableFileName) {
                ldLog() << "Adding default values to desktop file:" << path() << std::endl;

                auto rv = true;

                auto setDefault = [&rv, this](const std::string& section, const std::string& key, const std::string& value) {
                    if (entryExists(section, key)) {
                        std::string currentValue;
                        if (!getEntry(section, key, currentValue))
                            ldLog() << LD_ERROR << "This should never happen" << std::endl;

                        ldLog() << LD_WARNING << "Key exists, not modified:" << key << "(current value:" << currentValue << LD_NO_SPACE << ")" << std::endl;
                        rv = false;
                    } else {
                        if (setEntry(section, key, value)) {
                            // *should* be unreachable
                            rv = false;
                        }
                    }
                };

                setDefault("Desktop Entry", "Name", executableFileName);
                setDefault("Desktop Entry", "Exec", executableFileName);
                setDefault("Desktop Entry", "Icon", executableFileName);
                setDefault("Desktop Entry", "Type", "Application");
                setDefault("Desktop Entry", "Categories", "Utility;");

                return rv;
            }

            bool DesktopFile::validate() const {
                // FIXME: call desktop-file-validate
                return true;
            }

            bool DesktopFile::operator==(const DesktopFile& other) {
                return d->data == other.d->data;
            }

            bool DesktopFile::operator!=(const DesktopFile& other) {
                return !operator==(other);
            }
        }
    }
}



