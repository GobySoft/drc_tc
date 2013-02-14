#ifndef DRCTCGUI20130129H
#define DRCTCGUI20130129H

#include "config.pb.h"

#include "goby/common/liaison_container.h"

#include <zmq.hpp>

#include <Wt/WTimer>

extern "C"
{
    std::vector<goby::common::LiaisonContainer*> goby_liaison_load(
        const goby::common::protobuf::LiaisonConfig& cfg,
        boost::shared_ptr<zmq::context_t> zmq_context);
}

namespace drc_tc
{
    class DRCLiaisonGUI : public goby::common::LiaisonContainer
    {
      public:

        DRCLiaisonGUI(const DRCGUIConfig& cfg,
                      Wt::WContainerWidget* parent = 0);

        ~DRCLiaisonGUI()
        { }

      private:
        void create_layout();
        
        Wt::WContainerWidget* add_slider_box(const google::protobuf::FieldDescriptor* field,
                                             int min, int max,
                                             Wt::WContainerWidget* container,
                                             bool has_slider = true);
        void do_set_int32_value(int value, const google::protobuf::FieldDescriptor* field);
        void do_set_rate_enabled(bool enabled)
        { cfg_.set_do_rate_control(enabled); }
        
        void do_set_rate_units(int unit)
        { cfg_.set_rate_unit(static_cast<DRCGUIConfig::RateUnits>(unit)); }

        void do_set_tun_type(int unit)
        {
            cfg_.set_tunnel_type(static_cast<DRCGUIConfig::TunnelType>(unit));
            switch(static_cast<DRCGUIConfig::TunnelType>(unit))
            {
                case DRCGUIConfig::TCP_SERVER:
                    tcp_address_group_->setDisabled(true);
                    tcp_port_group_->setDisabled(false);
                    break;
                case DRCGUIConfig::TCP_CLIENT:
                    tcp_address_group_->setDisabled(false);
                    tcp_port_group_->setDisabled(false);
                    break;
                case DRCGUIConfig::LOOPBACK:
                    tcp_address_group_->setDisabled(true);
                    tcp_port_group_->setDisabled(true);
                    break;
            }
            
        }

        void do_set_tcp_address(Wt::WLineEdit* edit)
        {
            cfg_.set_tcp_address(edit->text().narrow());
        }        
        
        void do_set_tunnel_address(Wt::WLineEdit* edit)
        {
            cfg_.set_tunnel_address(edit->text().narrow());
        }        


        void do_remove();
        void do_apply();
        void do_change_tunnel();

        void tc_system(const std::stringstream& netem_command);

        void check_status();
        
        
      private:
        DRCGUIConfig cfg_;
        DRCGUIConfig last_cfg_;
        Wt::WVBoxLayout* main_layout_;
        Wt::WContainerWidget* container_;
        Wt::WPanel* status_panel_;
        Wt::WContainerWidget* status_group_;
        Wt::WGroupBox* current_group_;
        Wt::WText* pb_cfg_text_;
        Wt::WGroupBox* tc_group_;
        Wt::WText* tc_show_text_;
        Wt::WTimer status_timer_;
        Wt::WGroupBox* tcp_address_group_;
        Wt::WContainerWidget* tcp_port_group_;
        Wt::WPushButton* apply_;
        
        bool drc_tc_has_error_;

    };
}


#endif
