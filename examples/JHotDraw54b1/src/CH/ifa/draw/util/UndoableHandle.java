/*
 * @(#)UndoableHandle.java
 *
 * Project:		JHotdraw - a GUI framework for technical drawings
 *				http://www.jhotdraw.org
 *				http://jhotdraw.sourceforge.net
 * Copyright:	© by the original author(s) and all contributors
 * License:		Lesser GNU Public License (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

package CH.ifa.draw.util;

import CH.ifa.draw.framework.*;
import java.awt.*;

/**
 * @author  Wolfram Kaiser <mrfloppy@sourceforge.net>
 * @version <$CURRENT_VERSION$>
 */
public class UndoableHandle implements Handle {

	private Handle myWrappedHandle;
	private DrawingView myDrawingView;
	
	public UndoableHandle(Handle newWrappedHandle, DrawingView newDrawingView) {
		setWrappedHandle(newWrappedHandle);
		setDrawingView(newDrawingView);
	}
	
	/**
	 * Locates the handle on the figure. The handle is drawn
	 * centered around the returned point.
	 */
	public Point locate() {
		return getWrappedHandle().locate();
	}

	/**
	 * @deprecated As of version 4.1,
	 * use invokeStart(x, y, drawingView)
	 * Tracks the start of the interaction. The default implementation
	 * does nothing.
	 * @param x the x position where the interaction started
	 * @param y the y position where the interaction started
	 */
	public void invokeStart(int x, int y, Drawing drawing) {
		getWrappedHandle().invokeStart(x, y, drawing);
	}

	/**
	 * @deprecated As of version 4.1,
	 * use invokeStart(x, y, drawingView)
	 * Tracks the start of the interaction. The default implementation
	 * does nothing.
	 * @param x the x position where the interaction started
	 * @param y the y position where the interaction started
	 * @param view the handles container
	 */
	public void invokeStart(int x, int y, DrawingView view) {
		getWrappedHandle().invokeStart(x, y, view);
	}

	/**
	 * @deprecated As of version 4.1,
	 * use invokeStep(x, y, anchorX, anchorY, drawingView)
	 *
	 * Tracks a step of the interaction.
	 * @param dx x delta of this step
	 * @param dy y delta of this step
	 */
	public void invokeStep(int dx, int dy, Drawing drawing) {
		getWrappedHandle().invokeStep(dx, dy, drawing);
	}

	/**
	 * Tracks a step of the interaction.
	 * @param x the current x position
	 * @param y the current y position
	 * @param anchorX the x position where the interaction started
	 * @param anchorY the y position where the interaction started
	 */
	public void invokeStep(int x, int y, int anchorX, int anchorY, DrawingView view) {
		getWrappedHandle().invokeStep(x, y, anchorX, anchorY, view);
	}

	/**
	 * Tracks the end of the interaction.
	 * @param x the current x position
	 * @param y the current y position
	 * @param anchorX the x position where the interaction started
	 * @param anchorY the y position where the interaction started
	 */
	public void invokeEnd(int x, int y, int anchorX, int anchorY, DrawingView view) {
		getWrappedHandle().invokeEnd(x, y, anchorX, anchorY, view);

		Undoable undoableActivity = getWrappedHandle().getUndoActivity();
		if ((undoableActivity != null) && (undoableActivity.isUndoable())) {
			getDrawingView().editor().getUndoManager().pushUndo(undoableActivity);
			getDrawingView().editor().getUndoManager().clearRedos();
		}
	}

	/**
	 * @deprecated As of version 4.1,
	 * use invokeEnd(x, y, anchorX, anchorY, drawingView).
	 *
	 * Tracks the end of the interaction.
	 */
	public void invokeEnd(int dx, int dy, Drawing drawing) {
		getWrappedHandle().invokeEnd(dx, dy, drawing);
	}

	/**
	 * Gets the handle's owner.
	 */
	public Figure owner() {
		return getWrappedHandle().owner();
	}

	/**
	 * Gets the display box of the handle.
	 */
	public Rectangle displayBox() {
		return getWrappedHandle().displayBox();
	}

	/**
	 * Tests if a point is contained in the handle.
	 */
	public boolean containsPoint(int x, int y) {
		return getWrappedHandle().containsPoint(x, y);
	}

	/**
	 * Draws this handle.
	 */
	public void draw(Graphics g) {
		getWrappedHandle().draw(g);
	}

	protected void setWrappedHandle(Handle newWrappedHandle) {
		myWrappedHandle = newWrappedHandle;
	}
	
	protected Handle getWrappedHandle() {
		return myWrappedHandle;
	}

	public DrawingView getDrawingView() {
		return myDrawingView;
	}
	
	protected void setDrawingView(DrawingView newDrawingView) {
		myDrawingView = newDrawingView;
	}

	public Undoable getUndoActivity() {
		return new UndoableAdapter(getDrawingView());
	}

	public void setUndoActivity(Undoable newUndoableActivity) {
		// do nothing: always return default UndoableAdapter
	}
}
