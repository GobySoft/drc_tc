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
    
    {        
        std::stringstream netem_command;
        netem_command << "/sbin/tc qdisc replace dev tun" << cfg_.tunnel_num()
                      << " root handle 1:0 netem delay " << cfg_.latency_ms() << "ms"
                      << " loss " << cfg_.drop_percentage() << "%";
        
        tc_system(netem_command);
    }

    if(cfg_.do_rate_control())
    {
        std::stringstream netem_command;
        netem_command << "/sbin/tc qdisc replace dev tun" << cfg_.tunnel_num() << " parent 1:1 handle 10: tbf rate "
                      << cfg_.max_rate();

        switch(cfg_.rate_unit())
        {
            case DRCGUIConfig::BYTES_PER_SEC: netem_command << "bps"; break;
            case DRCGUIConfig::KBYTES_PER_SEC: netem_command << "kbps"; break;
            case DRCGUIConfig::MBYTES_PER_SEC: netem_command << "mbps"; break;
            case DRCGUIConfig::KBITS_PER_SEC: netem_command << "kbit"; break;
            case DRCGUIConfig::MBITS_PER_SEC: netem_command << "mbit"; break;
        }

        netem_command << " buffer 1600 limit 3000";
        
        tc_system(netem_command);
    }
    else
    {
        std::stringstream netem_command;
        netem_command << "/sbin/tc qdisc replace dev tun" << cfg_.tunnel_num() << " parent 1:1 pfifo";
        tc_system(netem_command);
    }
    
    quit();
}

void drc_tc::DrcTcApply::tc_system(const std::stringstream& netem_command)
{
    glog.is(DEBUG1, lock) && glog << "System: " << netem_command.str() <<  std::endl << unlock;    
    int rc = system(netem_command.str().c_str());
    if(rc)
    {
        glog.is(WARN, lock) && glog << "System returned code: " << rc <<  std::endl << unlock;
    }
    
}
