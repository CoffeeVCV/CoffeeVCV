#pragma once
#include <rack.hpp>

using namespace rack;

struct CoffeeSwitch2PosVert : app::SvgSwitch {
	CoffeeSwitch2PosVert() {
		shadow->opacity = 0.0;
		momentary=false;
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/Components/CoffeeSwitch2PosVert1.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/Components/CoffeeSwitch2PosVert2.svg")));
	}
};


struct CoffeeSlider : app::SvgSlider {
	CoffeeSlider() {
		setBackgroundSvg(Svg::load(asset::plugin(pluginInstance,"res/Components/CoffeeSlider.svg")));
		setHandleSvg(Svg::load(asset::plugin(pluginInstance,"res/Components/CoffeeSliderHandle.svg")));
		setHandlePosCentered(
			math::Vec(19.84260/2, 76.53517 - 11.74218/2),
			math::Vec(19.84260/2, 0.0 + 11.74218/2)
		);
	}
};

struct CoffeeKnob4mm : app::SvgKnob {
	widget::SvgWidget* bg;

	CoffeeKnob4mm() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		shadow->opacity=0;
		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);

		setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeKnob4mm.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeKnob4mm_bg.svg")));
	}
};

struct CoffeeKnob6mm : app::SvgKnob {
	widget::SvgWidget* bg;

	CoffeeKnob6mm() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		shadow->opacity=0;
		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);

		setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeKnob6mm.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeKnob6mm_bg.svg")));
	}
};

struct CoffeeKnob8mm : app::SvgKnob {
	widget::SvgWidget* bg;

	CoffeeKnob8mm() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		shadow->opacity=0;
		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);

		setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeKnob8mm.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeKnob8mm_bg.svg")));
	}
};

struct CoffeeKnob10mm : app::SvgKnob {
	widget::SvgWidget* bg;

	CoffeeKnob10mm() {
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		shadow->opacity=0;
		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);

		setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeKnob10mm.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance,"res/components/CoffeeKnob10mm_bg.svg")));
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
struct Coffee3mmButton : app::SvgSwitch {
	Coffee3mmButton() {
		momentary = true;
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/components/Coffee3mmButton1.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/components/Coffee3mmButton2.svg")));
	}
};
struct Coffee4mmButton : app::SvgSwitch {
	Coffee4mmButton() {
		momentary = true;
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/components/Coffee4mmButton1.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance,"res/components/Coffee4mmButton2.svg")));
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
