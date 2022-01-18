
#include "InstallFile.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>


namespace autom {

    TargetInstallRule::TargetInstallRule():targets(){
        type = Target;
    }

    FileInstallRule::FileInstallRule():files(){
        type = File;
    }

struct InstallFileSerializer::ReaderPriv {
    std::ifstream in;
    std::unique_ptr<rapidjson::IStreamWrapper> inputStreamWrapper;
    rapidjson::Document doc;
    
    rapidjson::Document::ValueIterator it;
    
    unsigned objectCount = 0;
    ReaderPriv():in(),inputStreamWrapper(nullptr){
        
    }
};

void InstallFileSerializer::DestroyReaderPriv::operator()(ReaderPriv *pt){
    delete pt;
}

void InstallFileSerializer::beginRead(const autom::StrRef & input){
//    std::cout << "INSTALL_FILE:" << input.data() << std::endl;
//    std::cout.flush();
    reader->in.open(input.data(),std::ios::in);
    if(!reader->in.is_open()){
        std::cout << "FAILED TO READ FROM:" << input.data() << std::endl;
        std::cout.flush();
        exit(-1);
    }
    
    reader->inputStreamWrapper.reset(new rapidjson::IStreamWrapper(reader->in));
    reader->doc.ParseStream(*reader->inputStreamWrapper);
    assert(reader->doc.IsArray() && "AUTOMINSTALL file is not properly formatted");
    reader->it = reader->doc.Begin();
}

bool InstallFileSerializer::getRule(InstallRulePtr & rule){
    if(reader->it == reader->doc.End()){
        return false;
    }
    auto & item = *reader->it;
    if(item.IsObject()){
        auto obj = item.GetObj();
        std::string target_type = obj["type"].GetString();
        if(target_type == "target"){
            rule = InstallRulePtr(new TargetInstallRule());
            rule->type = InstallRule::Target;
            auto _t = std::dynamic_pointer_cast<TargetInstallRule>(rule);
            auto & targets = _t->targets;
            auto _array = obj["targets"].GetArray();
            for(auto & obj : _array){
                auto target = new Target();
                target->name = new eval::String(std::string(obj.GetString(),obj.GetStringLength()));
                targets.emplace_back(target);
            }
            
        }
        else if(target_type == "file"){
            rule = InstallRulePtr(new FileInstallRule());
            rule->type = InstallRule::File;
            auto _t = std::dynamic_pointer_cast<FileInstallRule>(rule);
            auto & sources = _t->files;
            auto _array = obj["sources"].GetArray();
            for(auto & obj : _array){
                sources.push_back(std::string(obj.GetString(),obj.GetStringLength()));
            }
        }
        rule->prefixed_dest = obj["dest"].GetString();
        ++reader->it;
        return true;
    }
    else {
        return false;
    }
}

void InstallFileSerializer::endRead(){
    reader->in.close();
    reader->inputStreamWrapper.reset();
}

struct InstallFileSerializer::WriterPriv {
    std::ofstream out;
    rapidjson::OStreamWrapper outWrapper;
    rapidjson::PrettyWriter<decltype(outWrapper)> writer;
    WriterPriv():out(),outWrapper(out),writer(outWrapper){
        
    }
};

void InstallFileSerializer::DestroyWriterPriv::operator()(WriterPriv *pt){
    delete pt;
}

void InstallFileSerializer::beginWrite(const autom::StrRef & output){
    writer->out.open(output.data(),std::ios::out);
    writer->writer.StartArray();
}

void InstallFileSerializer::writeRule(InstallRulePtr rule){
    writer->writer.StartObject();
    writer->writer.Key("type",4);
    if(rule->type == InstallRule::Target){
        auto target_rule = std::dynamic_pointer_cast<TargetInstallRule>(rule);
        writer->writer.String("target",6);
        writer->writer.Key("targets",7);
        writer->writer.StartArray();
        for(auto & t : target_rule->targets){
            assert(t->type != FS_ACTION || t->type != SCRIPT_TARGET || t->type == GROUP_TARGET && "Only compiled targets can be given install time rules");
            std::filesystem::path full_target_path;
            if(t->type & COMPILED_OUTPUT_TARGET){
                auto _t = std::dynamic_pointer_cast<CompiledTarget>(t);
                full_target_path = std::filesystem::path(_t->output_dir->value().data()).append(_t->name->value().data());
                if(!_t->output_ext->empty()){
                    full_target_path.concat(".").concat(_t->output_ext->value().data());
                }
            }
            else {
                
            }
            auto str = full_target_path.string();
            writer->writer.String(str.data(),(rapidjson::SizeType)str.size());
        }
        writer->writer.EndArray();
    }
    else{
        auto file_rule = std::dynamic_pointer_cast<FileInstallRule>(rule);
        writer->writer.String("file",4);
        writer->writer.Key("sources",7);
        writer->writer.StartArray();
        for(auto & s : file_rule->files){
            writer->writer.String(s.data(),(rapidjson::SizeType)s.size());
        }
        writer->writer.EndArray();
    }
    writer->writer.Key("dest",4);
    writer->writer.String(rule->prefixed_dest.data(),(rapidjson::SizeType)rule->prefixed_dest.size());
    writer->writer.EndObject();
}

void InstallFileSerializer::endWrite(){
    writer->writer.EndArray();
    writer->out.close();
}

InstallFileSerializer::InstallFileSerializer():reader(new ReaderPriv(),DestroyReaderPriv()),writer(new WriterPriv(),DestroyWriterPriv()){
    
};


}
