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

import java.io.InputStream;
import java.io.Serializable;
import CH.ifa.draw.util.Storable;
import CH.ifa.draw.util.StorableInput;
import CH.ifa.draw.util.StorableOutput;

/**
 * ResourceContentProducer produces contents from resource in the application's
 * CLASSPATH.<br>
 * It takes a resource name and loads the resource as a String.
 *
 * It can either be specific if set for a specific resource, or generic, retrieving
 * any resource passed to the getContents method.
 *
 * @author    Eduardo Francos - InContext
 * @created   1 mai 2002
 * @version   1.0
 */

public class ResourceContentProducer extends AbstractContentProducer
		 implements Serializable {
	/** Description of the Field */
	protected String fResourceName = null;


	/**Constructor for the ResourceContentProducer object */
	public ResourceContentProducer() { }


	/**
	 *Constructor for the ResourceContentProducer object
	 *
	 * @param resourceName  Description of the Parameter
	 */
	public ResourceContentProducer(String resourceName) {
		fResourceName = resourceName;
	}


	/**
	 * Gets the content attribute of the ResourceContentProducer object
	 *
	 * @param context       Description of the Parameter
	 * @param ctxAttrName   Description of the Parameter
	 * @param ctxAttrValue  Description of the Parameter
	 * @return              The content value
	 */
	public Object getContent(ContentProducerContext context, String ctxAttrName, Object ctxAttrValue) {
		try {
			// if we have our own resource then use it
			// otherwise use the one supplied
			String resourceName = (fResourceName != null) ? fResourceName : (String)ctxAttrValue;

			InputStream reader = this.getClass().getResourceAsStream(resourceName);
			int available = reader.available();
			byte contents[] = new byte[available];
			reader.read(contents, 0, available);
			reader.close();
			return new String(contents);
		}
		catch (Exception ex) {
			ex.printStackTrace();
			return ex.toString();
		}
	}


	/**
	 * Writes the storable
	 *
	 * @param dw  the storable output
	 */
	public void write(StorableOutput dw) {
		super.write(dw);
		dw.writeString(fResourceName);
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
		fResourceName = (String)dr.readString();
	}
}
