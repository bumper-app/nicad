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

import java.io.IOException;

import java.io.Serializable;
import CH.ifa.draw.util.Storable;
import CH.ifa.draw.util.StorableInput;
import CH.ifa.draw.util.StorableOutput;

/**
 * FigureDataContentProducer produces content on behalf of Figures.<br>
 * It supports the basic information common to all figures.
 * It can only be used as a generic producer, getting information from any
 * figure passed to the getContents method.
 *
 * @author    Eduardo Francos - InContext
 * @created   30 avril 2002
 * @version   1.0
 */

public class FigureDataContentProducer extends AbstractContentProducer
		 implements Serializable {

	/**Constructor for the FigureContentProducer object */
	public FigureDataContentProducer() { }


	/**
	 * Produces the contents for the figure
	 *
	 * @param context       the calling client context
	 * @param ctxAttrName   the attribute name
	 * @param ctxAttrValue  the figure
	 * @return              The string value for the requested entity name
	 */
	public Object getContent(ContentProducerContext context, String ctxAttrName, Object ctxAttrValue) {
		if (ctxAttrName.compareTo(ContentProducer.ENTITY_FIGURE_WIDTH) == 0) {
			return Integer.toString(((FigureContentProducerContext)context).displayBox().width);
		}

		if (ctxAttrName.compareTo(ContentProducer.ENTITY_FIGURE_HEIGHT) == 0) {
			return Integer.toString(((FigureContentProducerContext)context).displayBox().height);
		}

		if (ctxAttrName.compareTo(ContentProducer.ENTITY_FIGURE_POSX) == 0) {
			return Integer.toString(((FigureContentProducerContext)context).displayBox().x);
		}

		if (ctxAttrName.compareTo(ContentProducer.ENTITY_FIGURE_POSY) == 0) {
			return Integer.toString(((FigureContentProducerContext)context).displayBox().y);
		}

		return null;
	}


	/**
	 * Writes the storable
	 *
	 * @param dw  the storable output
	 */
	public void write(StorableOutput dw) {
		super.write(dw);
	}


	/**
	 * Writes the storable
	 *
	 * @param dr               the storable input
	 * @exception IOException  thrown by called methods
	 */
	public void read(StorableInput dr)
		throws IOException {
		super.read(dr);
	}
}
