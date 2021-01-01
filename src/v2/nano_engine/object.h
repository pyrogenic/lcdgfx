/*
    MIT License

    Copyright (c) 2018-2020, Alexey Dynda

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
/**
 * @file sprite.h Sprite class
 */

#ifndef _NANO_OBJECT_H_
#define _NANO_OBJECT_H_

#include "canvas/point.h"
#include "canvas/rect.h"
#include "lcd_hal/io.h"
#include "tiler.h"

/**
 * @ingroup NANO_ENGINE_API_V2
 * @{
 */

/**
 * Template class for NanoEngine objects lists
 */
template<class T, class TItem>
class NanoObjectList;

/**
 * This is base class for all NanoObjects.
 */
template<class T>
class NanoObject: public NanoEngineObject<T>
{
public:
    template<class N, class P> friend class NanoObjectList;
    /**
     * Creates basic object with the size [1,1]
     *
     * @param pos location of the NanoObject being created
     */
    explicit NanoObject(const NanoPoint &pos)
        : m_rect{pos, pos}
    {
    }

    /**
     * Creates basic object with arbitrary size
     *
     * @param pos location of the NanoObject being created
     * @param size desirable size of NanoObject being created
     */
    NanoObject(const NanoPoint &pos, const NanoPoint &size)
        : m_rect{pos, pos + size - (NanoPoint){1,1}}
    {
    }

    virtual ~NanoObject(){};

    /**
     * Draws nano object Engine canvas
     */
    void draw() override { };

    /**
     * Marks nano object location for refreshing on the new frame
     */
    void refresh() override
    {
        if (this->hasTiler())
        {
             this->getTiler().refreshWorld( m_rect );
        }
    }

    /**
     * Updates NanoObject
     */
    void update() override { }

    /**
     * Returns width of NanoObject
     */
    lcdint_t width() const
    {
        return m_rect.width();
    }

    /**
     * Returns height of NanoObject
     */
    lcdint_t height() const
    {
        return m_rect.height();
    }

    /**
     * Moves sprite to new position and marks screen area for refresh
     */
    void moveTo(const NanoPoint &p)
    {
        refresh();
        setPos( p );
        refresh();
    }

    /**
     * Moves sprite to new position by specified offset
     */
    void moveBy(const NanoPoint &p)
    {
        refresh();
        setPos( m_rect.p1 + p );
        refresh();
    }

    /**
     * Resizes NanoObject and marks screen area for refresh.
     *
     * @param size new size of NanoObject
     */
    void resize(const NanoPoint &size)
    {
        refresh();
        setSize( size );
        refresh();
    }

    /**
     * Sets new size of NanoObject. Doesn't update screen content
     *
     * @param size new size of NanoObject
     */
    void setSize(const NanoPoint &size)
    {
        m_rect.p2.x = m_rect.p1.x + size.x - 1;
        m_rect.p2.y = m_rect.p1.y + size.y - 1;
    }

    /**
     * Sets position of NanoObject, doesn't mark for update content on the screen
     */
    void setPos(const NanoPoint &p)
    {
        m_rect = (NanoRect){ p,
                   (NanoPoint){ (lcdint_t)(p.x + m_rect.p2.x - m_rect.p1.x),
                                (lcdint_t)(p.y + m_rect.p2.y - m_rect.p1.y) } };
    }

    /**
     * Returns bottom-center point of the sprite
     */
    const NanoPoint bottom() const
    {
        return { (lcdint_t)((m_rect.p1.x + m_rect.p2.x) >> 1), m_rect.p2.y  };
    }

    /**
     * Returns top-center point of the sprite
     */
    const NanoPoint top() const
    {
        return { (lcdint_t)((m_rect.p1.x + m_rect.p2.x) >> 1), m_rect.p1.y  };
    }

    /**
     * Returns left-center point of the sprite
     */
    const NanoPoint left() const
    {
        return { m_rect.p1.x, (lcdint_t)((m_rect.p1.y + m_rect.p2.y) >> 1)  };
    }

    /**
     * Returns right-center point of the sprite
     */
    const NanoPoint right() const
    {
        return { m_rect.p2.x, (lcdint_t)((m_rect.p1.y + m_rect.p2.y) >> 1)  };
    }

    /**
     * Returns center point of the sprite
     */
    const NanoPoint center() const
    {
        return { (lcdint_t)((m_rect.p1.x + m_rect.p2.x) >> 1),
                 (lcdint_t)((m_rect.p1.y + m_rect.p2.y) >> 1) };
    }

    /**
     * Returns sprite x position
     */
    lcdint_t x( ) const { return m_rect.p1.x; }

    /**
     * Returns sprite y position
     */
    lcdint_t y( ) const { return m_rect.p1.y; }

    /**
     * Returns current sprite position (top-left corner)
     */
    const NanoPoint & getPosition() const { return m_rect.p1; }

    /**
     * Returns rectangle area, occupied by the NanoObject
     */
    const NanoRect & getRect() const { return m_rect; }

protected:
    /** Rectangle area occupied by the object */
    NanoRect       m_rect;
};

template<class T, class TItem = NanoObject<T>>
class NanoObjectList: public NanoObject<T>
{
public:
    using NanoObject<T>::NanoObject;

    typedef TItem value_type;
    /**
     * Returns next object in the list.
     * If no objects left, the function returns nullptr.
     *
     * @param prev previous object. If nullptr, then first object is returned.
     */
    value_type *getNext(value_type *prev = nullptr)
    {
        return static_cast<value_type*>(prev ? prev->m_next : m_first);
    }

    /**
     * Returns previous object in the list.
     * If no objects left, the function returns nullptr.
     *
     * @param curr current object. If nullptr, then last object is returned.
     */
    value_type *getPrev(value_type *curr = nullptr)
    {
        value_type *p = m_first;
        while (p)
        {
            if (p->m_next == curr)
            {
                break;
            }
            p = static_cast<value_type*>(p->m_next);
        }
        return p;
    }

    /**
     * Updates all objects in the list.
     */
    void update() override
    {
        value_type *p = getNext();
        while (p)
        {
            p->update();
            p = getNext( p );
        }
    }

    /**
     * Refreshes all objects in the list.
     */
    void refresh() override
    {
        value_type *p = getNext();
        while (p)
        {
            p->setTiler( this->m_tiler );
            p->refresh();
            p = getNext( p );
        }
    }

    /**
     * Draw all objects from the list in the buffer.
     */
    void draw() override
    {
        value_type *p = getNext();
        while (p)
        {
            p->draw();
            p = getNext(p);
        }
    }

    /**
     * Returns true if NanoObjectList contains specified object.
     *
     * @param object object to search for.
     */
    bool has(value_type *object) {
      value_type *p = getNext();
      while (p && p != object) {
        p = getNext(p);
      }
      return p != nullptr;
    }

    /**
     * Adds new NanoObject to the end of the list and marks it for refresh.
     *
     * @param object object to add.
     * @return true if object exists and was newly-added
     */
    bool add(value_type *object) {
      if (object == nullptr || has(object)) {
        return false;
      }
      object->m_next = nullptr;
      object->setTiler(this->m_tiler);
      if (!m_first) {
        m_first = object;
      } else {
        getPrev()->m_next = object;
      }
      object->refresh();
      return true;
    }

    /**
     * Adds new NanoObject to the beginning of the list and marks it for refresh.
     *
     * @param object object to insert.
     * @return true if object exists and was newly-added
     */
    bool insert(value_type *object) {
      if (object == nullptr || has(object)) {
        return false;
      }
      object->m_next = m_first;
      object->m_tiler = this->m_tiler;
      m_first = object;
      object->refresh();
      return true;
    }

    /**
     * Removes NanoObject from the list and marks occupied area for refresh.
     *
     * @param object object to remove.
     * @return true if object was in the list
     */
    bool remove(value_type *object)
    {
        if ( m_first == nullptr || object == nullptr )
        {
            return false;
        }
        else if ( object == m_first )
        {
            object->refresh();
            m_first = static_cast<value_type *>(object->m_next);
            object->m_next = nullptr;
            object->m_tiler = nullptr;
            return true;
        }
        else
        {
            value_type *p = m_first;
            while ( p->m_next )
            {
                if ( p->m_next == object )
                {
                    object->refresh();
                    p->m_next = object->m_next;
                    object->m_next = nullptr;
                    object->m_tiler = nullptr;
                    return true;
                }
                p = static_cast<value_type *>(p->m_next);
            }
        }
        return false;
    }

private:
    value_type *m_first = nullptr;
};

/**
 * @}
 */

#endif

