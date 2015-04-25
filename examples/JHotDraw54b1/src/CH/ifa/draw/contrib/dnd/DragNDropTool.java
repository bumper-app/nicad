/*
 * @(#)DragNDropTool.java
 *
 * Project:		JHotdraw - a GUI framework for technical drawings
 *				http://www.jhotdraw.org
 *				http://jhotdraw.sourceforge.net
 * Copyright:	© by the original author(s) and all contributors
 * License:		Lesser GNU Public License (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

package CH.ifa.draw.contrib.dnd;

import CH.ifa.draw.standard.AbstractTool;
import java.util.List;
import java.awt.event.MouseEvent;
import java.awt.*;
import java.util.Iterator;

import CH.ifa.draw.framework.*;
import CH.ifa.draw.standard.*;
import CH.ifa.draw.util.CollectionsFactory;

/**
 * This is a tool which handles drag and drop between Components in
 * JHotDraw and drags from JHotDraw.  It also indirectly
 * handles management of Drops from extra-JVM sources.
 *
 *
 * Drag and Drop is about information moving, not images or objects.  Its about
 * moving a JHD rectangle to another application and that application understanding
 * both its shape, color, attributes, and everything about it. not how it looks.
 *
 * There can be only 1 such tool in an application.  A view can be registered
 * with only a single DropSource as far as I know (maybe not).
 *
 * @todo    For intra JVM transfers we need to pass Point origin as well, and not
 * assume it will be valid which currently will cause a null pointer exception.
 * or worse, will be valid with some local value.
 * The dropSource will prevent simultaneous drops.
 *
 * For a Container to be initialized to support Drag and Drop, it must first
 * have a connection to a heavyweight component.  Or more precisely it must have
 * a peer.  That means new Component() is not capable of being initiated until
 * it has attachment to a top level component i.e. JFrame.add(comp);  If you add
 * a Component to a Container, that Container must be the child of some
 * Container which is added in its heirachy to a topmost Component.  I will
 * refine this description with more appropriate terms as I think of new ways to
 * express this.
 *
 * note: if drop target is same as dragsource then we should draw the object.
 *
 *
 * @author C.L.Gilbert <dnoyeb@sourceforge.net>
 * @version <$CURRENT_VERSION$>
 */
public class DragNDropTool extends AbstractTool {
	private Tool            fChild;
	private List            comps;

	public DragNDropTool(DrawingEditor editor) {
		super(editor);
		comps = CollectionsFactory.current().createList();
	}

	/**
	 * Sent when a new view is created
	 */
	public void viewCreated(DrawingView view) {
		super.viewCreated(view);
		if (DNDInterface.class.isInstance(view)) {
			DNDInterface dndi = (DNDInterface)view;
			dndi.setDropTargetActive(true);
			dndi.setDragSourceActive(false);
			comps.add(dndi);
		}
	}

	/**
	 * Send when an existing view is about to be destroyed.
	 */
	public void viewDestroying(DrawingView view) {
		if (DNDInterface.class.isInstance(view)) {
			DNDInterface dndi = (DNDInterface)view;
			dndi.setDropTargetActive(false);
			dndi.setDragSourceActive(false);
			comps.remove(dndi);
		}
		super.viewDestroying(view);
	}

	/**
	 * Turn on drag by adding a DragGestureRegognizer to all Views which are
	 * based on Components.
	 */
	public void activate() {
		super.activate();
		System.out.println("DNDTool Activation");

		setDragSourceActive(true);
	}

	public void deactivate() {
		System.out.println("DNDTool deactivation.");
		setDragSourceActive(false);
		super.deactivate();
	}

	private void setDragSourceActive(boolean newState) {
		Iterator it = comps.iterator();
		while (it.hasNext()) {
			DNDInterface dndi = (DNDInterface)it.next();
			dndi.setDragSourceActive(newState);
		}
	}

	/**
	 * Sets the type of cursor based on what is under the coordinates in the
	 * active view.
	 */
	public static void setCursor(int x, int y, DrawingView view) {
		if (view == null) {   //shouldnt need this
			return;
		}
		Handle handle = view.findHandle(x, y);
		Figure figure = view.drawing().findFigure(x, y);

		if (handle != null) {
			if (LocatorHandle.class.isInstance(handle)) {
				LocatorHandle lh = (LocatorHandle)handle;
				Locator loc = lh.getLocator();
				if (RelativeLocator.class.isInstance(loc)) {
					RelativeLocator rl = (RelativeLocator) loc;
					if (rl.equals( RelativeLocator.north())) {
						view.setCursor(new Cursor(Cursor.N_RESIZE_CURSOR));
					}
					else if (rl.equals(RelativeLocator.northEast())) {
						view.setCursor(new Cursor(Cursor.NE_RESIZE_CURSOR));
					}
					else if (rl.equals(RelativeLocator.east())) {
						view.setCursor(new Cursor(Cursor.E_RESIZE_CURSOR));
					}
					else if (rl.equals(RelativeLocator.southEast())) {
						view.setCursor(new Cursor(Cursor.SE_RESIZE_CURSOR));
					}
					else if (rl.equals(RelativeLocator.south())) {
						view.setCursor(new Cursor(Cursor.S_RESIZE_CURSOR));
					}
					else if (rl.equals(RelativeLocator.southWest())) {
						view.setCursor(new Cursor(Cursor.SW_RESIZE_CURSOR));
					}
					else if (rl.equals(RelativeLocator.west())) {
						view.setCursor(new Cursor(Cursor.W_RESIZE_CURSOR));
					}
					else if (rl.equals(RelativeLocator.northWest())) {
						view.setCursor(new Cursor(Cursor.NW_RESIZE_CURSOR));
					}
				}
			}
		}
		else if (figure != null) {
			view.setCursor(new Cursor(Cursor.MOVE_CURSOR));
		}
		else {
			view.setCursor(Cursor.getDefaultCursor());
		}
	}

	/**
	 * Handles mouse moves (if the mouse button is up).
	 * Switches the cursors depending on whats under them.
     * Don't use x, y use getX and getY so get the real unlimited position
	 * Part of the Tool interface.
	 */
	public void mouseMove(MouseEvent evt, int x, int y) {
		if (evt.getSource() == getActiveView()) {
			setCursor(evt.getX(), evt.getY(), getActiveView());
		}
	}

	/**
	 * Handles mouse up events. The events are forwarded to the
	 * current tracker.
	 * Part of the Tool interface.
	 */
	public void mouseUp(MouseEvent e, int x, int y) {
		if (fChild != null) { // JDK1.1 doesn't guarantee mouseDown, mouseDrag, mouseUp
			fChild.mouseUp(e, x, y);
		}
		fChild = null;
		view().unfreezeView();
	}

	/**
	 * Handles mouse down events and starts the corresponding tracker.
	 * Part of the Tool interface.
	 */
	public void mouseDown(MouseEvent e, int x, int y) {
		super.mouseDown(e, x, y);
		// on MS-Windows NT: AWT generates additional mouse down events
		// when the left button is down && right button is clicked.
		// To avoid dead locks we ignore such events
		if (fChild != null) {
			return;
		}

		view().freezeView();

		Handle handle = view().findHandle(e.getX(), e.getY());
		if (handle != null) {
			fChild = createHandleTracker(handle);
		}
		else {
			Figure figure = drawing().findFigure(e.getX(), e.getY());
			if (figure != null) {
				//fChild = createDragTracker(editor(), figure);
				//fChild.activate();
				fChild = null;
				if (e.isShiftDown()) {
				   view().toggleSelection(figure);
				}
				else if (!view().isFigureSelected(figure)) {
					view().clearSelection();
					view().addToSelection(figure);
				}
			}
			else {
				if (!e.isShiftDown()) {
					view().clearSelection();
				}
				fChild = createAreaTracker();
			}
		}
		if (fChild != null) {
			fChild.mouseDown(e, x, y);
		}
	}

	/**
	 * Handles mouse drag events. The events are forwarded to the
	 * current tracker.
	 * Part of the Tool interface.
	 */
	public void mouseDrag(MouseEvent e, int x, int y) {
		if (fChild != null) { // JDK1.1 doesn't guarantee mouseDown, mouseDrag, mouseUp
			fChild.mouseDrag(e, x, y);
		}
	}

	/**
	 * Factory method to create an area tracker. It is used to select an
	 * area.
	 */
	protected Tool createAreaTracker() {
		return new SelectAreaTracker(editor());
	}

	/**
	 * Factory method to create a Drag tracker. It is used to drag a figure.
	 */
	protected Tool createDragTracker(DrawingEditor editor, Figure f) {
		return new DragTracker(editor, f);
	}

	/**
	 * Factory method to create a Handle tracker. It is used to track a handle.
	 */
	protected Tool createHandleTracker(Handle handle) {
		return new HandleTracker(editor(), handle);
	}
}
