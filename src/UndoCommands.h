#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <QUndoCommand>
#include <QScopedPointer>
#include "GraphicsItems.h"

class Category;

class DrawCommand : public QUndoCommand
{
public:
    DrawCommand(CanvasItem* canvas, Drawable* shape, QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;

private:
    CanvasItem* canvas_;
    const Category* category_;
    QScopedPointer<Drawable> shape_;
    QScopedPointer<CanvasItem::Fragment> fragment_;
};


class DrawFragmentCommand : public QUndoCommand
{
public:
    DrawFragmentCommand(CanvasItem* canvas, CanvasItem::Fragment* original, QUndoCommand *parent = 0);

    void undo() override;
    void redo() override;

private:
    CanvasItem* canvas_;
    QScopedPointer<CanvasItem::Fragment> shape_;
    QScopedPointer<CanvasItem::Fragment> original_;
};

#endif // UNDOCOMMANDS_H
