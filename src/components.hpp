#pragma once
#include <rack.hpp>

using namespace rack;

struct CoffeeTrimpot : app::SvgKnob {
	widget::SvgWidget* bg;

	CoffeeTrimpot() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		shadow->opacity=0;
		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);

		setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeTrimpot.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeTrimpot_bg.svg")));
	}
};

template <typename TBase = GrayModuleLightWidget>
struct TOrangeLight : TBase {
	TOrangeLight() {
		this->addBaseColor(SCHEME_ORANGE);
	}
};
using OrangeLight = TOrangeLight<>;

struct CoffeeTinyButton : app::SvgSwitch {
	CoffeeTinyButton() {
		momentary = true;
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeTinyButton1.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeTinyButton2.svg")));
	}
};

struct CoffeeButtonVertIndicator : app::SvgSwitch {
	CoffeeButtonVertIndicator() {
		momentary = true;
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeButtonVertIndicator1.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeButtonVertIndicator2.svg")));
	}
};

struct CoffeeTinyButtonLatch : CoffeeTinyButton {
	CoffeeTinyButtonLatch() {
		momentary = false;
		latch = true;
	}
};

template <typename TBase>
struct CoffeeTinyLight : TSvgLight<TBase> {
	CoffeeTinyLight() {
		this->setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeTinyLight.svg")));
	}
};

template <typename TBase = GrayModuleLightWidget>
struct CoffeeTinySimpleLight : TBase {
	CoffeeTinySimpleLight() {
		this->box.size = mm2px(math::Vec(2, 2));
	}
};


struct CoffeeOutputPort : app::SvgPort {
	CoffeeOutputPort() {
		shadow->opacity=0;
		setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeOutputPort.svg")));
	}
};

struct CoffeeOutputPortIndicator : app::SvgPort {
	CoffeeOutputPortIndicator() {
		shadow->opacity=0;
		setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeOutputPortIndicator.svg")));
	}
};

struct CoffeeOutputPortButton : app::SvgPort {
	CoffeeOutputPortButton() {
		shadow->opacity=0;
		setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeOutputPortButton.svg")));
	}
};

struct CoffeeInputPort : app::SvgPort {
	CoffeeInputPort() {
		shadow->opacity=0;
		setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeInputPort.svg")));
	}
};

struct CoffeeInputPortIndicator : app::SvgPort {
	CoffeeInputPortIndicator() {
		shadow->opacity=0;
		setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeInputPortIndicator.svg")));
	}
};

struct CoffeeInputPortButton : app::SvgPort {
	CoffeeInputPortButton() {
		shadow->opacity=0;
		setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeInputPortButton.svg")));
	}
};
