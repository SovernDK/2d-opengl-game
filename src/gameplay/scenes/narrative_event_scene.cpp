#pragma once
#include "scenes/narrative_event_scene.h"

#include "SDL3/SDL.h"

#include "core/game.h"
#include "core/texts.h"
#include "services/ui_service.h"

#include "rmui/ui_anim.h"
#include "rmui/ui_widget.h"

#include "imgui.h"
#include "scenes/loading_scene.h"
#include "graphics/rendering/canvas_2d.h"

#include "utility/file_util.h"

using namespace rmui;
using namespace ecs;

std::shared_ptr<UIWidget> bookWindow, leftPage, rightPage, header, content, choices;
std::shared_ptr<UIButton> choice1, choice2, choice3;

NarrativeEventScene::~NarrativeEventScene() {}

void NarrativeEventScene::start(core::IContext* ctx)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
	auto ui = ServiceLocator::get<IUIService>();

	if(!Resources::textureExist("open_book"))
		Resources::loadTexture(file_util::createPath("assets", "open_book.png").string(), "open_book");
	if (!Resources::textureExist("landscape"))
	{
		Resources::loadTexture(file_util::createPath("assets", "landscape.png").string(), "landscape");
		bgTex = Resources::texture("open_book")->id;
	}

	bookWindow = ui->createWindow()
		.setLocPos(0.525f, 0.49f)
		.setLocSize(0.83f, 0.86f)
		.setPivot(UIAnchor::Center)
		.setStyle("book_window")
		.setAlpha(0)
		.build("book_window");

	auto leftPage = ui->createWindow()
		.setPivot(UIAnchor::Top_Left)
		.setStyle("page_window")
		.setParent(bookWindow)
		.setAlpha(0)
		.build("left_page_win");

	auto rightPage = ui->createWindow()
		.setPivot(UIAnchor::Top_Right)
		.setStyle("page_window")
		.setParent(bookWindow)
		.setAlpha(0)
		.build("right_page_win");

	auto header = ui->createImage()
		.setPivot(UIAnchor::Top_Left)
		.setStyle("page_header")
		.setParent(rightPage)
		.setImage(Resources::texture("landscape")->id)
		.build("right_page_header");

	const auto loremIpsum = "Lorem ipsum dolor sit amet, \nconsectetur adipiscing elit. \nQuisque et condimentum augue. \nInteger pharetra eget purus sed tincidunt. \nAliquam a erat nisi.";
	auto content = ui->createMultiLabel()
		.setPivot(UIAnchor::Top_Left)
		.setParent(rightPage)
		.setText(loremIpsum)
		.build("right_page_content");
	
	// Choices
	auto choices = ui->createWindow()
		.setStyle("page_window")
		.setParent(rightPage)
		.setAlpha(0)
		.build("right_page_choices");

	auto choice1 = ui->createButton()
		.setStyle("choice_button")
		.setParent(choices)
		.setText("Lorem ipsum dolor sit amet")
		.setAlpha(0)
		.build("choice1");

	auto choice2 = ui->createButton()
		.setStyle("choice_button")
		.setParent(choices)
		.setText("Quisque et condimentum augue")
		.setAlpha(0)
		.build("choice2");

	auto choice3 = ui->createButton()
		.setStyle("choice_button")
		.setParent(choices)
		.setText("Gallia est omnis divisas in partes tres")
		.setAlpha(0)
		.build("choice3");

	choice1->label->play<FadeIn>(0.5f);
	choice2->label->play<FadeIn>(0.5f);
	choice3->label->play<FadeIn>(0.5f);
}

void NarrativeEventScene::update(core::IContext* ctx, float dt)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
}

void NarrativeEventScene::draw(core::IContext* ctx)
{
	core::Game& game = static_cast<core::Game&>(*ctx);

	Canvas2D::setDepth(BACKGROUND_Z);
	Canvas2D::setColor(WHITE);
	Canvas2D::setBlend(BlendMode::Alpha);
	Canvas2D::drawImage(bgTex, { 0, 0, 1920, 1080 }, uvs);
	Canvas2D::reset();
}

void NarrativeEventScene::unload(core::IContext* ctx)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
	auto ui = ServiceLocator::get<IUIService>();

	ui->destroy("book_window");
}

void NarrativeEventScene::quit(core::IContext* ctx)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
	auto ui = ServiceLocator::get<IUIService>();
}