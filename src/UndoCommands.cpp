#include "UndoCommands.h"
#include "Drawables.h"

DrawCommand::DrawCommand(CanvasItem* canvas, Drawable* shape, QUndoCommand *parent)
	: QUndoCommand(parent)
	, canvas_(canvas)
	, category_(canvas->category())
	, shape_(shape)
	, fragment_(canvas->extractFragment(shape->rect()))
{
}

void DrawCommand::undo()
{
	canvas_->restoreFragment(*fragment_);
}

void DrawCommand::redo()
{
	auto c = canvas_->category();
	canvas_->setCategory(category_);
	canvas_->draw(*shape_);
	canvas_->setCategory(c);
}

//-----------------------------------------------------------------------------------

DrawFragmentCommand::DrawFragmentCommand(CanvasItem* canvas, CanvasItem::Fragment* original, QUndoCommand* parent)
	: QUndoCommand(parent)
	, canvas_(canvas)
	, shape_(canvas->extractFragment(original->rect(), canvas->category()))
	, original_(original)
{
}

void DrawFragmentCommand::undo()
{
	canvas_->restoreFragment(*original_);
}

void DrawFragmentCommand::redo()
{
	canvas_->restoreFragment(*shape_);
}
