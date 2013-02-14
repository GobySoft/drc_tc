#include "goby/common/application_base.h"

#include "config.pb.h"

using namespace goby::common::logger;
using goby::common::logger_lock::lock;
using goby::glog;


namespace drc_tc
{
    
    class DrcTcApply : public goby::common::ApplicationBase
    {
    public:
        DrcTcApply();
        
    private:
        void iterate() { }
        void tc_system(const std::stringstream& netem_command);
    private:
        static DRCGUIConfig cfg_;
        
    };
}

DRCGUIConfig drc_tc::DrcTcApply::cfg_;

int main(int argc, char* argv[])
{
    goby::run<drc_tc::DrcTcApply>(argc, argv);
}
                            
drc_tc::DrcTcApply::DrcTcApply() :
    ApplicationBase(&cfg_)
{
    const google::protobuf::Descriptor* desc = DRCGUIConfig::descriptor();

    std::cout << "#!/bin/bash" << std::endl;
    for(int i = 0, n = desc->field_count(); i < n; ++i)
    {
        std::cout << "DRC_TC_" << boost::to_upper_copy(desc->field(i)->name())
                  << "=";

        std::string value;
        google::protobuf::TextFormat::PrintFieldValueToString(cfg_, desc->field(i), -1, &value);
        std::cout << value << std::endl;
    }

    quit();
    
}

