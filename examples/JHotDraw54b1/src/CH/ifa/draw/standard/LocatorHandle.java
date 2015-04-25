/*
 * @(#)LocatorHandle.java
 *
 * Project:		JHotdraw - a GUI framework for technical drawings
 *				http://www.jhotdraw.org
 *				http://jhotdraw.sourceforge.net
 * Copyright:	© by the original author(s) and all contributors
 * License:		Lesser GNU Public License (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

package CH.ifa.draw.standard;

import java.awt.Point;
import CH.ifa.draw.framework.*;

/**
 * A LocatorHandle implements a Handle by delegating the location requests to
 * a Locator object.
 *
 * @see Locator
 *
 * @version <$CURRENT_VERSION$>
 */
public class LocatorHandle extends AbstractHandle {

	private Locator       fLocator;

	/**
	 * Initializes the LocatorHandle with the given Locator.
	 */
	public LocatorHandle(Figure owner, Locator l) {
		super(owner);
		fLocator = l;
	}
	/**
	 * This should be cloned or it gives the receiver the opportunity to alter
	 * our internal behavior.
	 */
	public Locator getLocator() {
		return fLocator;
	}

	/**
	 * Locates the handle on the figure by forwarding the request
	 * to its figure.
	 */
	public Point locate() {
		return fLocator.locate(owner());
	}
}
