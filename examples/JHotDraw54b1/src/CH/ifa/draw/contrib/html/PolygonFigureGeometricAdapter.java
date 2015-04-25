/*
 *  @(#)TextAreaFigure.java
 *
 *  Project:		JHotdraw - a GUI framework for technical drawings
 *  http://www.jhotdraw.org
 *  http://jhotdraw.sourceforge.net
 *  Copyright:	© by the original author(s) and all contributors
 *  License:		Lesser GNU Public License (LGPL)
 *  http://www.opensource.org/licenses/lgpl-license.html
 */
package CH.ifa.draw.contrib.html;

import java.awt.Point;
import java.awt.Polygon;
import java.awt.Shape;

import java.awt.geom.AffineTransform;
import java.awt.geom.PathIterator;
import java.util.Iterator;
import CH.ifa.draw.contrib.PolygonFigure;

/**
 * Geometric adapter for the PolygonFigure
 *
 * @author    Eduardo Francos - InContext
 * @created   3 mai 2002
 * @version   1.0
 */

public class PolygonFigureGeometricAdapter extends PolygonFigure
		 implements GeometricFigure {

	/**Constructor for the PolyLineFigureGeometricAdapter object */
	public PolygonFigureGeometricAdapter() {
		super();
	}


	/**
	 *Constructor for the PolyLineFigureGeometricAdapter object
	 *
	 * @param x  Description of the Parameter
	 * @param y  Description of the Parameter
	 */
	public PolygonFigureGeometricAdapter(int x, int y) {
		super(x, y);
	}


	/**
	 *Constructor for the PolyLineFigureGeometricAdapter object
	 *
	 * @param p  Description of the Parameter
	 */
	public PolygonFigureGeometricAdapter(Polygon p) {
		super(p);
	}


	/**
	 * Gets the shape attribute of the PolygonFigure object
	 *
	 * @return   The shape value
	 */
	public Shape getShape() {
		return getInternalPolygon();
	}

}
