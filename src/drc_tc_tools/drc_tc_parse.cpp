#include "goby/common/application_base.h"

#include "config.pb.h"

using namespace goby::common::logger;
using goby::glog;


namespace drc_tc
{
    
    class DrcTcApply : public goby::common::ApplicationBase
    {
    public:
        DrcTcApply(DRCGUIConfig* cfg);
        
    private:
        void iterate() { }
        void tc_system(const std::stringstream& netem_command);
    private:
        DRCGUIConfig& cfg_;
        
    };
}


int main(int argc, char* argv[])
{
    DRCGUIConfig cfg;
    goby::run<drc_tc::DrcTcApply>(argc, argv, &cfg);
}
                            
drc_tc::DrcTcApply::DrcTcApply(DRCGUIConfig* cfg) :
    ApplicationBase(cfg),
    cfg_(*cfg)
{
    const google::protobuf::Descriptor* desc = DRCGUIConfig::descriptor();

    std::cout << "#!/bin/bash -e" << std::endl;
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

