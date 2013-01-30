#ifndef DRCTCGUI20130129H
#define DRCTCGUI20130129H

#include "config.pb.h"

#include "goby/common/liaison_container.h"

#include <zmq.hpp>

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
        {
            do_remove();
        }

      private:
        Wt::WContainerWidget* add_slider_box(const google::protobuf::FieldDescriptor* field,
                                             int min, int max,
                                             Wt::WContainerWidget* container);
        void do_set_int32_value(int value, const google::protobuf::FieldDescriptor* field);
        void do_set_rate_enabled(bool enabled)
        { cfg_.set_do_rate_control(enabled); }
        
        void do_set_rate_units(int unit)
        { cfg_.set_rate_unit(static_cast<DRCGUIConfig::RateUnits>(unit)); }
        
        void do_remove();
        void do_apply();

        void tc_system(const std::stringstream& netem_command);
        
      private:
        DRCGUIConfig cfg_;
        Wt::WVBoxLayout* main_layout_;
        Wt::WContainerWidget* container_;
        Wt::WGroupBox* current_group_;
        Wt::WText* current_text_;
    };
}


#endif
