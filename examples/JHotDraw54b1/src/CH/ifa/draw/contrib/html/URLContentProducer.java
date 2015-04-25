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
import java.net.URL;
import CH.ifa.draw.util.Storable;
import CH.ifa.draw.util.StorableInput;
import CH.ifa.draw.util.StorableOutput;

/**
 * URLContentProducer produces string contents from URLs.<br>
 * Anything the <code>URL.openStream()</code> method can get can be retrieved
 * by this producer, that includes resources, local files, web documents,
 * web queries, FTP files, and you name it.<br>
 * It can either be specific if set for a specific URL, or generic, retrieving
 * any URL passed to the getContents method.
 *
 * @author    Eduardo Francos - InContext
 * @created   4 mai 2002
 * @version   1.0
 * @todo      should we cache the contents for specific URLs? this can
 * accelerate things a lot for static documents, but for dynamic ones it
 * will complicate things. If cached then if must be in a DisposableResourceHolder
 */

public class URLContentProducer extends FigureDataContentProducer
		 implements Serializable {
	/** the specific URL */
	protected URL fURL = null;


	/**Constructor for the URLContentProducer object */
	public URLContentProducer() { }


	/**
	 *Constructor for the URLContentProducer object
	 *
	 * @param url  the specific URL
	 */
	public URLContentProducer(URL url) {
		fURL = url;
	}


	/**
	 * Retrieves the contents of the URL pointed object
	 *
	 * @param context       the calling client context
	 * @param ctxAttrName   the attribute name that led to this being called
	 * @param ctxAttrValue  the value of the URL attribute
	 * @return              the contents of the URL pointed object as a string
	 */
	public Object getContent(ContentProducerContext context, String ctxAttrName, Object ctxAttrValue) {
		try {
			// if we have our own URL then use it
			// otherwise use the one supplied
			URL url = (fURL != null) ? new URL(fURL.toExternalForm()) : new URL(((URL)ctxAttrValue).toExternalForm());

			InputStream reader = url.openStream();
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
		dw.writeBoolean((fURL != null));
		if (fURL != null) {
			dw.writeString(fURL.toExternalForm());
		}
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
		boolean hasURL = dr.readBoolean();
		if (hasURL) {
			fURL = new URL(dr.readString());
		}
	}
}
