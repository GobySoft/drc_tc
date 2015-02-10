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
    {
        std::stringstream netem_command;
        netem_command << "/sbin/tc qdisc delete dev tun" << cfg_.tunnel_num() << "  root";
        tc_system(netem_command);
    }
    
    
    if(cfg_.do_rate_control())
    {
        std::stringstream netem_command;
        netem_command << "/sbin/tc qdisc add dev tun" << cfg_.tunnel_num() << "  root handle 1:0 tbf rate "
                      << cfg_.max_rate();

        
        switch(cfg_.rate_unit())
        {
            case DRCGUIConfig::BYTES_PER_SEC: netem_command << "bps"; break;
            case DRCGUIConfig::KBYTES_PER_SEC: netem_command << "kbps"; break;
            case DRCGUIConfig::MBYTES_PER_SEC: netem_command << "mbps"; break;
            case DRCGUIConfig::KBITS_PER_SEC: netem_command << "kbit"; break;
            case DRCGUIConfig::MBITS_PER_SEC: netem_command << "mbit"; break;
        }

        const int use_hz = 100;
        netem_command << " buffer ";

        int buffer_size = 0;
        switch(cfg_.rate_unit())
        {
            case DRCGUIConfig::BYTES_PER_SEC: buffer_size = cfg_.max_rate() / use_hz; break;
            case DRCGUIConfig::KBYTES_PER_SEC: buffer_size = 1000*cfg_.max_rate() / use_hz; break;
            case DRCGUIConfig::MBYTES_PER_SEC: buffer_size =  1000000*cfg_.max_rate() / use_hz; break;
            case DRCGUIConfig::KBITS_PER_SEC: buffer_size =  1000/8*cfg_.max_rate() / use_hz; break;
            case DRCGUIConfig::MBITS_PER_SEC: buffer_size =  1000000/8*cfg_.max_rate() / use_hz; break;
        }
        if(buffer_size < 1600)
            buffer_size = 1600;
        
        // trying to get the right buffer size is tricky. Adding this in to reduce dropped packets 
        buffer_size *= 2;
        
        netem_command << buffer_size;
        netem_command << " latency 10ms";
        
        tc_system(netem_command);
    }

    std::string hierarchy = " root handle 1:0";
    if(cfg_.do_rate_control())
    {
        hierarchy = " parent 1:1 handle 10:";
    }

    if(cfg_.latency_ms() > 0 || cfg_.drop_percentage() > 0)
    {            
        std::stringstream netem_command;
        netem_command << "/sbin/tc qdisc add dev tun" << cfg_.tunnel_num()
                      << hierarchy << " netem delay " << cfg_.latency_ms() << "ms"
                      << " loss " << cfg_.drop_percentage() << "%";
        tc_system(netem_command);
    }


    
    quit();
}

void drc_tc::DrcTcApply::tc_system(const std::stringstream& netem_command)
{
    glog.is(DEBUG1) && glog << "System: " << netem_command.str() <<  std::endl;    
    int rc = system(netem_command.str().c_str());
    if(rc)
    {
        glog.is(WARN) && glog << "System returned code: " << rc <<  std::endl;
    }
    
}
