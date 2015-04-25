/*
 * @(#)Handle.java
 *
 * Project:		JHotdraw - a GUI framework for technical drawings
 *				http://www.jhotdraw.org
 *				http://jhotdraw.sourceforge.net
 * Copyright:	© by the original author(s) and all contributors
 * License:		Lesser GNU Public License (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

package CH.ifa.draw.framework;

import CH.ifa.draw.util.Undoable;
import java.awt.*;

/**
 * Handles are used to change a figure by direct manipulation.
 * Handles know their owning figure and they provide methods to
 * locate the handle on the figure and to track changes.
 * <hr>
 * <b>Design Patterns</b><P>
 * <img src="images/red-ball-small.gif" width=6 height=6 alt=" o ">
 * <b><a href=../pattlets/sld004.htm>Adapter</a></b><br>
 * Handles adapt the operations to manipulate a figure to a common interface.
 *
 * @see Figure
 *
 * @version <$CURRENT_VERSION$>
 */
public interface Handle {

	public static final int HANDLESIZE = 8;

	/**
	 * Locates the handle on the figure. The handle is drawn
	 * centered around the returned point.
	 */
	public Point locate();

	/**
	 * @deprecated As of version 4.1,
	 * use invokeStart(x, y, drawingView)
	 * Tracks the start of the interaction. The default implementation
	 * does nothing.
	 * @param x the x position where the interaction started
	 * @param y the y position where the interaction started
	 */
	public void invokeStart(int  x, int  y, Drawing drawing);

	/**
	 * @deprecated As of version 4.1,
	 * use invokeStart(x, y, drawingView)
	 * Tracks the start of the interaction. The default implementation
	 * does nothing.
	 * @param x the x position where the interaction started
	 * @param y the y position where the interaction started
	 * @param view the handles container
	 */
	public void invokeStart(int  x, int  y, DrawingView view);

	/**
	 * @deprecated As of version 4.1,
	 * use invokeStep(x, y, anchorX, anchorY, drawingView)
	 *
	 * Tracks a step of the interaction.
	 * @param dx x delta of this step
	 * @param dy y delta of this step
	 */
	public void invokeStep (int dx, int dy, Drawing drawing);

	/**
	 * Tracks a step of the interaction.
	 * @param x the current x position
	 * @param y the current y position
	 * @param anchorX the x position where the interaction started
	 * @param anchorY the y position where the interaction started
	 */
	public void invokeStep (int x, int y, int anchorX, int anchorY, DrawingView view);

	/**
	 * Tracks the end of the interaction.
	 * @param x the current x position
	 * @param y the current y position
	 * @param anchorX the x position where the interaction started
	 * @param anchorY the y position where the interaction started
	 */
	public void invokeEnd(int x, int y, int anchorX, int anchorY, DrawingView view);

	/**
	 * @deprecated As of version 4.1,
	 * use invokeEnd(x, y, anchorX, anchorY, drawingView).
	 *
	 * Tracks the end of the interaction.
	 */
	public void invokeEnd  (int dx, int dy, Drawing drawing);

	/**
	 * Gets the handle's owner.
	 */
	public Figure owner();

	/**
	 * Gets the display box of the handle.
	 */
	public Rectangle displayBox();

	/**
	 * Tests if a point is contained in the handle.
	 */
	public boolean containsPoint(int x, int y);

	/**
	 * Draws this handle.
	 */
	public void draw(Graphics g);

	public Undoable getUndoActivity();

	public void setUndoActivity(Undoable newUndoableActivity);
}
