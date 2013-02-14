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
#include <Wt/WLineEdit>
#include <Wt/WPanel>

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
      status_panel_(new Wt::WPanel(container_)),
      status_group_(new Wt::WContainerWidget),
      current_group_(new Wt::WGroupBox("Current settings", status_group_)),
      pb_cfg_text_(new WText("<pre>" + cfg.DebugString() + "</pre>", current_group_)),
      tc_group_(new Wt::WGroupBox("tc", status_group_)),
      tc_show_text_(new WText(tc_group_))
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

    set_name("DRCTrafficControl");
    create_layout();

    status_timer_.setInterval(1000);
    status_timer_.timeout().connect(this, &DRCLiaisonGUI::check_status);
    status_timer_.start(); 
}

void drc_tc::DRCLiaisonGUI::create_layout()
{
    status_panel_->setTitle("Shaper status");
    status_panel_->setCentralWidget(status_group_);
    status_panel_->setCollapsible(true);
    status_panel_->setPositionScheme(Wt::Fixed);
    status_panel_->setOffsets(20, Wt::Right | Wt::Bottom);
   

    
    do_remove();
    do_apply();
    
    main_layout_->addWidget(container_);    

    Wt::WGroupBox* update_box = new Wt::WGroupBox("Update settings: ", container_);

    Wt::WPushButton *apply = new Wt::WPushButton("Apply changes", update_box);
    apply->clicked().connect(this, &drc_tc::DRCLiaisonGUI::do_apply);
    new Wt::WBreak(update_box);
    new Wt::WBreak(update_box);


    
    const google::protobuf::Descriptor* desc = cfg_.GetDescriptor();
    add_slider_box(desc->FindFieldByNumber(1), 0, 5000, update_box); // latency_ms
    add_slider_box(desc->FindFieldByNumber(2), 0, 100, update_box); // drop_percentage

    new Wt::WText(desc->FindFieldByNumber(10)->name(),
                  update_box); // do rate control
    Wt::WCheckBox* rate_control_enabled = new Wt::WCheckBox(update_box);
    rate_control_enabled->setChecked(cfg_.do_rate_control());
    rate_control_enabled->checked().connect(boost::bind(&DRCLiaisonGUI::do_set_rate_enabled, this, true));
    rate_control_enabled->unChecked().connect(boost::bind(&DRCLiaisonGUI::do_set_rate_enabled, this, false));
    Wt::WContainerWidget* rate_group = add_slider_box(desc->FindFieldByNumber(11), 0, 1000, update_box); // max_rate

    if(!cfg_.do_rate_control()) rate_group->setDisabled(true);
    rate_control_enabled->checked().connect(boost::bind(&Wt::WContainerWidget::setDisabled, rate_group, false));
    rate_control_enabled->unChecked().connect(boost::bind(&Wt::WContainerWidget::setDisabled, rate_group, true));

    
    Wt::WComboBox* rate_unit_box = new Wt::WComboBox(rate_group);

    for(DRCGUIConfig::RateUnits i = DRCGUIConfig::RateUnits_MIN, n = DRCGUIConfig::RateUnits_MAX;
        i <= n; i = static_cast<DRCGUIConfig::RateUnits>(i + 1))
    {
        rate_unit_box->insertItem(i, DRCGUIConfig::RateUnits_Name(i));
    }
    rate_unit_box->setCurrentIndex(cfg_.rate_unit());
    rate_unit_box->activated().connect(this, &DRCLiaisonGUI::do_set_rate_units);
    rate_group->addWidget(new WBreak());
    rate_group->addWidget(new WBreak());
    
    add_slider_box(desc->FindFieldByNumber(100), 0, 100, update_box, false); // tunnel_num

    Wt::WGroupBox* tun_type_group = new Wt::WGroupBox(desc->FindFieldByNumber(101)->name(),
                                                      update_box); // tunnel_type
    
    Wt::WComboBox* tun_type_box = new Wt::WComboBox(tun_type_group);

    for(DRCGUIConfig::TunnelType i = DRCGUIConfig::TunnelType_MIN, n = DRCGUIConfig::TunnelType_MAX;
        i <= n; i = static_cast<DRCGUIConfig::TunnelType>(i + 1))
    {
        tun_type_box->insertItem(i, DRCGUIConfig::TunnelType_Name(i));
    }
    tun_type_box->setCurrentIndex(cfg_.tunnel_type());
    tun_type_box->activated().connect(this, &DRCLiaisonGUI::do_set_tun_type);

    Wt::WGroupBox* tcp_address_group = new Wt::WGroupBox(desc->FindFieldByNumber(102)->name(),
                                                         update_box); // tcp_address
    
    Wt::WLineEdit* tcp_address_edit = new Wt::WLineEdit(cfg_.tcp_address(), tcp_address_group);
    tcp_address_edit->changed().connect(boost::bind(&drc_tc::DRCLiaisonGUI::do_set_tcp_address, this, tcp_address_edit));
    
    add_slider_box(desc->FindFieldByNumber(103), 1024, 65535, update_box, false); // tcp_port
}


Wt::WContainerWidget* drc_tc::DRCLiaisonGUI::add_slider_box(const google::protobuf::FieldDescriptor* field, int min, int max, Wt::WContainerWidget* container, bool has_slider/* =true*/)
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

    if(has_slider)
    {
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
        slider->valueChanged().connect(boost::bind(&DRCLiaisonGUI::do_set_int32_value, this, _1, field));
        box->valueChanged().connect(slider, &Wt::WSlider::setValue);
    }
    
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
    pb_cfg_text_->setText("<pre>" + cfg_.DebugString() + "</pre>");

    if(cfg_.SerializeAsString() == last_cfg_.SerializeAsString())
    {
        glog.is(DEBUG1, lock) && glog << "No changes to config... ignoring apply" << std::endl << unlock;
        return;
    }
    
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

    if(cfg_.tunnel_type() != last_cfg_.tunnel_type() ||
       cfg_.tcp_address() != last_cfg_.tcp_address() ||
       cfg_.tcp_port() != last_cfg_.tcp_port())

    {
        do_change_tunnel();
    }
        
    {        
        std::stringstream command;
        command << "drc_tc_apply -c " << cfg_.this_config_location() << std::endl;        
        tc_system(command);
    }

    last_cfg_ = cfg_;
}


void drc_tc::DRCLiaisonGUI::do_change_tunnel()
{
    {        
        std::stringstream command;
        command << "service drc_tc restart" << std::endl;        
        tc_system(command);
    }
}


void drc_tc::DRCLiaisonGUI::check_status()
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
        int rc = pclose(fp);
        if(rc)
        {
            Wt::WCssDecorationStyle red_text;
            red_text.setForegroundColor(Wt::WColor(Wt::red));
            tc_show_text_->setDecorationStyle(red_text);
            std::stringstream ss;
            ss << "Error: for more details see /var/log/upstart/drc_tc.log" << std::endl;
            switch(cfg_.tunnel_type())
            {
                case DRCGUIConfig::TCP_SERVER:
                    ss << "Check that the port is not in use." << std::endl;
                    break;
                case DRCGUIConfig::TCP_CLIENT:
                    ss << "Check that server address is correct \nand that the server is running." << std::endl;
                    break;
                case DRCGUIConfig::LOOPBACK:
                    break;
            }
            tc_show_text_->setText("<pre>" + ss.str() + "</pre>");
            // so that we can retry even without changing settings.
            last_cfg_.Clear();
        }
        else
        {
            Wt::WCssDecorationStyle text;
            tc_show_text_->setDecorationStyle(text);
            tc_show_text_->setText("<pre> # " + netem_command.str() + "\n"
                                   + result.str() + "</pre>");
        }
    }        


//    Wt::WCssDecorationStyle gray_text;
//    gray_text.setForegroundColor(Wt::WColor(Wt::gray));
//    tc_show_text_->setDecorationStyle(gray_text);

}    
