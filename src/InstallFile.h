#include "Target.h"
#include <vector>
#include <memory>
#include <fstream>

#ifndef AUTOM_INSTALLFILE_H
#define AUTOM_INSTALLFILE_H

namespace autom {

struct InstallRule {
    typedef enum : int {
        Target,
        File,
    } Type;
    Type type;
    std::string prefixed_dest;
    virtual ~InstallRule() = default;
};

struct TargetInstallRule : public InstallRule {
    explicit TargetInstallRule();
    std::vector<std::shared_ptr<::autom::Target>> targets;
    ~TargetInstallRule() override = default;
};

struct FileInstallRule : public InstallRule {
    explicit FileInstallRule();
    std::vector<std::string> files;
    ~FileInstallRule() override = default;
};

typedef std::shared_ptr<InstallRule> InstallRulePtr;

class InstallFileSerializer {
    struct ReaderPriv;
    struct DestroyReaderPriv {
        void operator()(ReaderPriv *pt);
    };
    struct WriterPriv;
    struct DestroyWriterPriv {
        void operator()(WriterPriv *pt);
    };
    std::unique_ptr<ReaderPriv,DestroyReaderPriv> reader;
    std::unique_ptr<WriterPriv,DestroyWriterPriv> writer;
protected:
    void beginWrite(const autom::StrRef & output);
    void writeRule(InstallRulePtr rule);
    void endWrite();
    void beginRead(const autom::StrRef & input);
    bool getRule(  InstallRulePtr & rule);
    void endRead();
public:
    InstallFileSerializer();
};

}

#endif
