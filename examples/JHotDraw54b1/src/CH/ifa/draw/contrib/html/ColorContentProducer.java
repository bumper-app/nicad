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

import java.awt.Color;
import java.io.IOException;
import java.io.Serializable;
import CH.ifa.draw.util.Storable;
import CH.ifa.draw.util.StorableInput;
import CH.ifa.draw.util.StorableOutput;

/**
 * ColorContentProducer produces RGB color encoded hexadecimal strings.<br>
 * Ex:
 * <code>Color.blue ==> 0x0000FF</code><br>
 * <code>Color.red  ==> 0xFF00FF</code><br>
 * It can either be specific if set for a specific color, or generic, encoding
 * any color passed to the getContents method.
 *
 * @author    Eduardo Francos - InContext
 * @created   30 avril 2002
 * @version   1.0
 */

public class ColorContentProducer extends FigureDataContentProducer
		 implements Serializable {
	Color fColor = null;


	/**Constructor for the ColorContentProducer object */
	public ColorContentProducer() { }


	/**
	 *Constructor for the ColorContentProducer object
	 *
	 * @param color  the specific color to use
	 */
	public ColorContentProducer(Color color) {
		fColor = color;
	}


	/**
	 * Produces the contents for the color
	 *
	 * @param context       the calling client context
	 * @param ctxAttrName   the color attribute name (FrameColor, TextColor, etc)
	 * @param ctxAttrValue  the color
	 * @return              The string RBG value for the color
	 */
	public Object getContent(ContentProducerContext context, String ctxAttrName, Object ctxAttrValue) {
		// if we have our own color then use it
		// otherwise use the one supplied
		Color color = (fColor != null) ? fColor : (Color)ctxAttrValue;
		String colorCode = Integer.toHexString(color.getRGB());
		return "0x" + colorCode.substring(colorCode.length() - 6);
	}


	/**
	 * Sets the color attribute of the ColorContentProducer object
	 *
	 * @param color  The new color value
	 */
	public void setColor(Color color) {
		fColor = color;
	}


	/**
	 * Gets the color attribute of the ColorContentProducer object
	 *
	 * @return   The color value
	 */
	public Color getColor() {
		return fColor;
	}


	/**
	 * Writes the storable
	 *
	 * @param dw  the storable output
	 */
	public void write(StorableOutput dw) {

		super.write(dw);
		dw.writeBoolean((fColor != null));
		if (fColor != null) {
			dw.writeInt(fColor.getRGB());
		}
	}


	/**
	 * Reads the storable
	 *
	 * @param dr               the storable input
	 * @exception IOException  thrown by called methods
	 */
	public void read(StorableInput dr)
		throws IOException {

		super.read(dr);
		boolean hasColor = dr.readBoolean();
		if (hasColor) {
			fColor = new Color(dr.readInt());
		}
		else{
			fColor = null;
		}
	}
}
