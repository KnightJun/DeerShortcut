#include "DeerShortcut.h"
/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project.
** See the Qxt AUTHORS file for a list of authors and copyright holders.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the LibQxt project nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** <http://libqxt.org>  <foundation@libqxt.org>
*****************************************************************************/

#include "qxtglobalshortcut_p.h"

#include <QAbstractEventDispatcher>

#ifndef Q_OS_MAC
int QxtGlobalShortcutPrivate::ref = 0;
#   if QT_VERSION < QT_VERSION_CHECK(5,0,0)
QAbstractEventDispatcher::EventFilter QxtGlobalShortcutPrivate::prevEventFilter = 0;
#   endif
#endif // Q_OS_MAC
QHash<QPair<quint32, quint32>, DeerShortcut*> QxtGlobalShortcutPrivate::shortcuts;

QxtGlobalShortcutPrivate::QxtGlobalShortcutPrivate(DeerShortcut *q)
    : enabled(true)
    , key(Qt::Key(0))
    , mods(Qt::NoModifier)
    , nativeKey(0)
    , nativeMods(0)
    , registered(false)
    , q_ptr(q)
{
#ifndef Q_OS_MAC
    if (ref == 0) {
#   if QT_VERSION < QT_VERSION_CHECK(5,0,0)
        prevEventFilter = QAbstractEventDispatcher::instance()->setEventFilter(eventFilter);
#   else
        QAbstractEventDispatcher::instance()->installNativeEventFilter(this);
#endif
    }
    ++ref;
#endif // Q_OS_MAC
}

QxtGlobalShortcutPrivate::~QxtGlobalShortcutPrivate()
{
    unsetShortcut();

#ifndef Q_OS_MAC
    --ref;
    if (ref == 0) {
        QAbstractEventDispatcher *ed = QAbstractEventDispatcher::instance();
        if (ed != nullptr) {
#   if QT_VERSION < QT_VERSION_CHECK(5,0,0)
            ed->setEventFilter(prevEventFilter);
#   else
            ed->removeNativeEventFilter(this);
#   endif
        }
    }
#endif // Q_OS_MAC
}

bool QxtGlobalShortcutPrivate::setShortcut(const QKeySequence& shortcut)
{
    unsetShortcut();
    const Qt::KeyboardModifiers allMods =
            Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;
    const auto xkeyCode = static_cast<uint>( shortcut.isEmpty() ? 0 : shortcut[0] );
    // WORKAROUND: Qt has convert some keys to upper case which
    //             breaks some shortcuts on some keyboard layouts.
    const uint keyCode = QChar::toLower(xkeyCode & ~allMods);

    key = Qt::Key(keyCode);
    mods = Qt::KeyboardModifiers(xkeyCode & allMods);

    if (shortcut.isEmpty())return true;

    nativeKey = nativeKeycode(key);
    nativeMods = nativeModifiers(mods);

    registered = registerShortcut(nativeKey, nativeMods);
    if (registered) {
        shortcuts.insert(qMakePair(nativeKey, nativeMods), q_ptr);
    }
    else {
        key = Qt::Key(0);
        mods = Qt::KeyboardModifiers(0);
    }

    return registered;
}

bool QxtGlobalShortcutPrivate::unsetShortcut()
{
    if (registered
            && shortcuts.value(qMakePair(nativeKey, nativeMods)) == q_ptr
            && unregisterShortcut(nativeKey, nativeMods))
    {
        shortcuts.remove(qMakePair(nativeKey, nativeMods));
        registered = false;
        return true;
    }

    return false;
}

void QxtGlobalShortcutPrivate::activateShortcut(quint32 nativeKey, quint32 nativeMods)
{
    DeerShortcut* shortcut = shortcuts.value(qMakePair(nativeKey, nativeMods));
    if (shortcut && shortcut->isEnabled())
        emit shortcut->activated(shortcut);
}

/*!
    \class DeerShortcut
    \brief The DeerShortcut class provides a global shortcut aka "hotkey".

    A global shortcut triggers even if the application is not active. This
    makes it easy to implement applications that react to certain shortcuts
    still if some other application is active or if the application is for
    example minimized to the system tray.

    Example usage:
    \code
    DeerShortcut* shortcut = new DeerShortcut(window);
    connect(shortcut, SIGNAL(activated()), window, SLOT(toggleVisibility()));
    shortcut->setShortcut(QKeySequence("Ctrl+Shift+F12"));
    \endcode
 */

/*!
    \fn DeerShortcut::activated()

    This signal is emitted when the user types the shortcut's key sequence.

    \sa shortcut
 */

/*!
    Constructs a new DeerShortcut with \a parent.
 */
DeerShortcut::DeerShortcut(QObject* parent)
    : QObject(parent)
    , d_ptr(new QxtGlobalShortcutPrivate(this))
{
}

/*!
    Constructs a new DeerShortcut with \a shortcut and \a parent.
 */
DeerShortcut::DeerShortcut(const QKeySequence& shortcut, QObject* parent)
    : DeerShortcut(parent)
{
    setShortcut(shortcut);
}

/*!
    Destructs the DeerShortcut.
 */
DeerShortcut::~DeerShortcut()
{
    delete d_ptr;
}

/*!
    \property DeerShortcut::shortcut
    \brief the shortcut key sequence

    Notice that corresponding key press and release events are not
    delivered for registered global shortcuts even if they are disabled.
    Also, comma separated key sequences are not supported.
    Only the first part is used:

    \code
    qxtShortcut->setShortcut(QKeySequence("Ctrl+Alt+A,Ctrl+Alt+B"));
    Q_ASSERT(qxtShortcut->shortcut() == QKeySequence("Ctrl+Alt+A"));
    \endcode

    \sa setShortcut()
 */
QKeySequence DeerShortcut::shortcut() const
{
    return QKeySequence( static_cast<int>(d_ptr->key | d_ptr->mods) );
}

bool DeerShortcut::trySetShortcut(const QKeySequence& shortcut)
{
    QKeySequence bakHK = this->shortcut();
    bool ret = this->setShortcut(shortcut);
    if(!ret)this->setShortcut(bakHK);
    return ret;
}

/*!
    \property DeerShortcut::shortcut
    \brief sets the shortcut key sequence

    \sa shortcut()
 */
bool DeerShortcut::setShortcut(const QKeySequence& shortcut)
{
    bool ret = d_ptr->setShortcut(shortcut);
    if (ret || shortcut.isEmpty()) {
        shortcutSeted(shortcut);
    }
    return ret || shortcut.isEmpty();
}

/*!
    \property DeerShortcut::enabled
    \brief whether the shortcut is enabled

    A disabled shortcut does not get activated.

    The default value is \c true.

    \sa setEnabled(), setDisabled()
 */
bool DeerShortcut::isEnabled() const
{
    return d_ptr->enabled;
}

/*!
    \property DeerShortcut::valid
    \brief whether the shortcut was successfully set up
 */
bool DeerShortcut::isValid() const
{
    return d_ptr->registered;
}

/*!
    Sets the shortcut \a enabled.

    \sa enabled
 */
void DeerShortcut::setEnabled(bool enabled)
{
    d_ptr->enabled = enabled;
}

/*!
    Sets the shortcut \a disabled.

    \sa enabled
 */
void DeerShortcut::setDisabled(bool disabled)
{
    d_ptr->enabled = !disabled;
}
