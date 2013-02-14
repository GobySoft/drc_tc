#include <Wt/WText>
#include <Wt/WVBoxLayout>
#include <Wt/WBreak>
#include <Wt/WSlider>
#include <Wt/WSpinBox>
#include <Wt/WGroupBox>
#include <Wt/WComboBox>
#include <Wt/WCheckBox>
#include <Wt/WPushButton>
#include <Wt/WCssDecorationStyle>
#include <Wt/WColor>

#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "goby/common/logger.h"

#include "drc_tc_gui.h"

using namespace Wt;
using namespace goby::common::logger;
using goby::common::logger_lock::lock;
using goby::glog;

extern "C"
{
    std::vector<goby::common::LiaisonContainer*> goby_liaison_load(
        const goby::common::protobuf::LiaisonConfig& cfg,
        boost::shared_ptr<zmq::context_t> zmq_context)
    {
        goby::common::LiaisonContainer* container = new drc_tc::DRCLiaisonGUI(cfg.GetExtension(drc_gui_config));
        return std::vector<goby::common::LiaisonContainer*>(1, container);
    }
    
}

drc_tc::DRCLiaisonGUI::DRCLiaisonGUI(const DRCGUIConfig& cfg,
                                     Wt::WContainerWidget* parent)
    : LiaisonContainer(parent),
      cfg_(cfg),
      main_layout_(new Wt::WVBoxLayout(this)),
      container_(new Wt::WContainerWidget(this)),
      current_group_(new Wt::WGroupBox("Current settings: ", container_)),
      pb_cfg_text_(new WText("<pre>" + cfg.DebugString() + "</pre>", current_group_)),
      tc_show_text_(new WText(current_group_))
{
    // re-read the configuration upon construction
    std::ifstream cfg_file(cfg_.this_config_location().c_str());
    if(cfg_file.is_open())
    {
        google::protobuf::io::IstreamInputStream cfg_pb_stream(&cfg_file);
        google::protobuf::TextFormat::Parse(&cfg_pb_stream, &cfg_);
    }
    else
    {
        glog.is(WARN, lock) && glog << "Could not open file: " << cfg_.this_config_location() << " for reading back configuration changes." << std::endl;
    }

    

    Wt::WCssDecorationStyle gray_text;
    gray_text.setForegroundColor(Wt::WColor(Wt::gray));
    tc_show_text_->setDecorationStyle(gray_text);
    
    do_remove();
    do_apply();
    
    main_layout_->addWidget(container_);    

    Wt::WGroupBox* update_box = new Wt::WGroupBox("Update settings: ", container_);

    const google::protobuf::Descriptor* desc = cfg.GetDescriptor();
    add_slider_box(desc->FindFieldByNumber(1), 0, 5000, update_box); // latency_ms
    add_slider_box(desc->FindFieldByNumber(2), 0, 100, update_box); // drop_percentage
    Wt::WContainerWidget* rate_group = add_slider_box(desc->FindFieldByNumber(11), 0, 1000, update_box); // max_rate
    
    Wt::WComboBox* rate_unit_box = new Wt::WComboBox(rate_group);

    for(DRCGUIConfig::RateUnits i = DRCGUIConfig::RateUnits_MIN, n = DRCGUIConfig::RateUnits_MAX;
        i <= n; i = static_cast<DRCGUIConfig::RateUnits>(i + 1))
    {
        rate_unit_box->insertItem(i, DRCGUIConfig::RateUnits_Name(i));
    }
    rate_unit_box->setCurrentIndex(cfg.rate_unit());
    rate_unit_box->activated().connect(this, &DRCLiaisonGUI::do_set_rate_units);
    rate_group->addWidget(new WBreak());
    
    new Wt::WText("Rate control enabled: ", rate_group);
    Wt::WCheckBox* rate_control_enabled = new Wt::WCheckBox(rate_group);
    rate_control_enabled->setChecked(cfg.do_rate_control());
    rate_control_enabled->checked().connect(boost::bind(&DRCLiaisonGUI::do_set_rate_enabled, this, true));
    rate_control_enabled->unChecked().connect(boost::bind(&DRCLiaisonGUI::do_set_rate_enabled, this, false));
    
    rate_group->addWidget(new WBreak());

    new Wt::WBreak(update_box);
    Wt::WPushButton *apply = new Wt::WPushButton("Apply changes", update_box);
    apply->clicked().connect(this, &drc_tc::DRCLiaisonGUI::do_apply);
    
    set_name("DRCTrafficControl");
}


Wt::WContainerWidget* drc_tc::DRCLiaisonGUI::add_slider_box(const google::protobuf::FieldDescriptor* field,
                                     int min, int max, Wt::WContainerWidget* container)
{
    const std::string& name = field->name();
    
    Wt::WGroupBox* group = new Wt::WGroupBox(name, container);
    int start = cfg_.GetReflection()->GetInt32(cfg_, field);

    
    Wt::WSpinBox *box = new Wt::WSpinBox(group);
    box->setNativeControl(true);
    box->setMinimum(min);
    box->setMaximum(max);
    box->setValue(start);
    box->setInline(false);
    
    Wt::WSlider *slider = new Wt::WSlider(Wt::Horizontal, group);
    slider->setNativeControl(true);
    slider->setMinimum(min);
    slider->setMaximum(max);
    slider->setValue(start);
    slider->setTickInterval((max-min) / 10);
    slider->setTickPosition(Wt::WSlider::TicksBothSides);
    slider->resize(400, 50);
    slider->setInline(false);

    slider->valueChanged().connect(box, &Wt::WSpinBox::setValue);
    box->valueChanged().connect(slider, &Wt::WSlider::setValue);
    
    slider->valueChanged().connect(boost::bind(&DRCLiaisonGUI::do_set_int32_value, this, _1, field));
    box->valueChanged().connect(boost::bind(&DRCLiaisonGUI::do_set_int32_value, this, _1, field));
    
    return group;
}


void drc_tc::DRCLiaisonGUI::do_set_int32_value(int value, const google::protobuf::FieldDescriptor* field)
{
    cfg_.GetReflection()->SetInt32(&cfg_, field, value);
}

void drc_tc::DRCLiaisonGUI::tc_system(const std::stringstream& netem_command)
{
    glog.is(DEBUG1, lock) && glog << "System: " << netem_command.str() <<  std::endl << unlock;    
    int rc = system(netem_command.str().c_str());
    if(rc)
    {
        glog.is(WARN, lock) && glog << "System returned code: " << rc <<  std::endl << unlock;
    }
    
}
        

void drc_tc::DRCLiaisonGUI::do_remove()
{
    std::stringstream netem_command;
    netem_command << "/sbin/tc qdisc del dev tun" << cfg_.tunnel_num() << " root";
    
    tc_system(netem_command);
}


void drc_tc::DRCLiaisonGUI::do_apply()
{
    glog.is(DEBUG1, lock) && glog << "Cfg: " << cfg_.DebugString() <<  std::endl << unlock;

    std::ofstream cfg_file(cfg_.this_config_location().c_str());
    if(cfg_file.is_open())
    {        
        cfg_file << "# This file is overwritten using the Goby Liaison GUI DRCTrafficControl tab" << std::endl;
        cfg_file << cfg_.DebugString() << std::endl;
        cfg_file.close();
    }
    else
    {
        glog.is(WARN, lock) && glog << "Could not open file: " << cfg_.this_config_location() << " for writing back configuration changes." << std::endl;
    }
    
    {        
        std::stringstream command;
        command << "drc_tc_apply -c " << cfg_.this_config_location() << std::endl;        
        tc_system(command);
    }
    {
        // grab output of tc show
        FILE *fp;
        int status;
        char output[100];
        
        /* Open the command for reading. */
        std::stringstream netem_command, result;
        netem_command << "/sbin/tc qdisc show dev tun" << cfg_.tunnel_num();
        fp = popen(netem_command.str().c_str(), "r");
        if (fp)
        {
            /* Read the output a line at a time - output it. */
            while (fgets(output, sizeof(output)-1, fp) != NULL) {
                result << output;
            }
            /* close */
            pclose(fp);
        }        

        pb_cfg_text_->setText("<pre>" + cfg_.DebugString() + "</pre>");
        tc_show_text_->setText("<pre> # " + netem_command.str() + "\n"
                               + result.str() + "</pre>");
    }    

    
}

