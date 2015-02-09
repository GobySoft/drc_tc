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
        {
            cfg_.set_do_rate_control(enabled);
            apply_->setDisabled(false);
        }
        
        void do_set_rate_units(int unit)
        {
            cfg_.set_rate_unit(static_cast<DRCGUIConfig::RateUnits>(unit));
            apply_->setDisabled(false);
        }

        void do_set_tun_type(int unit)
        {
            cfg_.set_tunnel_type(static_cast<DRCGUIConfig::TunnelType>(unit));
            switch(static_cast<DRCGUIConfig::TunnelType>(unit))
            {
                case DRCGUIConfig::LOOPBACK:
                    remote_address_group_->setDisabled(true);
                    port_group_->setDisabled(true);
                    remote_tunnel_address_group_->setDisabled(true);
                    break;
                case DRCGUIConfig::OPENVPN:
                    remote_address_group_->setDisabled(false);
                    port_group_->setDisabled(false);
                    remote_tunnel_address_group_->setDisabled(false);
                    break;
            }
            apply_->setDisabled(false);
        }

        void do_set_remote_address(Wt::WLineEdit* edit)
        {
            cfg_.set_remote_address(edit->text().narrow());
            apply_->setDisabled(false);
        }        
        
        void do_set_tunnel_address(Wt::WLineEdit* edit)
        {
            cfg_.set_tunnel_address(edit->text().narrow());
            apply_->setDisabled(false);
        }        

        void do_set_remote_tunnel_address(Wt::WLineEdit* edit)
        {
            cfg_.set_remote_tunnel_address(edit->text().narrow());
            apply_->setDisabled(false);
        }        


        void do_set_link2_remote_address(Wt::WLineEdit* edit)
        {
            cfg_.set_link2_remote_address(edit->text().narrow());
            apply_->setDisabled(false);
        }        
        
        void do_set_link2_tunnel_address(Wt::WLineEdit* edit)
        {
            cfg_.set_link2_tunnel_address(edit->text().narrow());
            apply_->setDisabled(false);
        }        
        
        void do_set_link2_blackout_file(Wt::WLineEdit* edit)
        {
            cfg_.set_link2_blackout_file(edit->text().narrow());
            apply_->setDisabled(false);
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
        Wt::WGroupBox* ip_group_;
        Wt::WText* ip_show_text_;
        Wt::WTimer status_timer_;
        Wt::WGroupBox* tunnel_address_group_;
        Wt::WGroupBox* remote_tunnel_address_group_;
        Wt::WGroupBox* remote_address_group_;
        Wt::WContainerWidget* port_group_;
        Wt::WPushButton* apply_;
        
        Wt::WGroupBox* link2_tunnel_address_group_;
        Wt::WGroupBox* link2_remote_address_group_;
        Wt::WGroupBox* link2_blackout_file_group_;


        bool drc_tc_has_error_;

    };
}


#endif
