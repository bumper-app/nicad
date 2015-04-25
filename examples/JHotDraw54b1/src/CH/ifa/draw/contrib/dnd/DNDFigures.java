/*
 * @(#)DNDFigures.java
 *
 * Project:		JHotdraw - a GUI framework for technical drawings
 *				http://www.jhotdraw.org
 *				http://jhotdraw.sourceforge.net
 * Copyright:	© by the original author(s) and all contributors
 * License:		Lesser GNU Public License (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

package CH.ifa.draw.contrib.dnd;

import CH.ifa.draw.framework.FigureEnumeration;
import CH.ifa.draw.standard.FigureEnumerator;
import CH.ifa.draw.util.CollectionsFactory;

import java.awt.Point;
import java.util.List;

/**
 * @author  C.L.Gilbert <dnoyeb@sourceforge.net>
 * @version <$CURRENT_VERSION$>
 */
public class DNDFigures implements java.io.Serializable {
	private List figures;
	private Point origin;

	public DNDFigures(FigureEnumeration fe, Point origin) {
		this.figures = CollectionsFactory.current().createList();
		// copy figure enumeration because enumerations should not be fields
		while (fe.hasNextFigure()) {
			figures.add(fe.nextFigure());
		}
		this.origin = origin;
	}

	public FigureEnumeration getFigures() {
	    return new FigureEnumerator(figures);
	}

	public Point getOrigin() {
	    return origin;
	}
}