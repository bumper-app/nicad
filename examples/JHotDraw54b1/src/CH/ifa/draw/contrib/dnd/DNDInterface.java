/*
 * @(#)DNDInterface.java
 *
 * Project:		JHotdraw - a GUI framework for technical drawings
 *				http://www.jhotdraw.org
 *				http://jhotdraw.sourceforge.net
 * Copyright:	© by the original author(s) and all contributors
 * License:		Lesser GNU Public License (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

package CH.ifa.draw.contrib.dnd;

import java.awt.dnd.DnDConstants;
import java.awt.datatransfer.*;

/**
 * Interface for Components which wish to participate in drag and drop.
 *
 * A Component which wishes to participate in drag and drop should implement
 * this interface.  Once done the DragBDropTool will be able to transfer data
 * to and from the Component.  The Component will also be activated to receive
 * drops from extra-JVM sources according to the conditions the Component
 * specifies.
 *
 * @author  C.L.Gilbert <dnoyeb@sourceforge.net>
 * @version <$CURRENT_VERSION$>
 */
public interface DNDInterface {

	public boolean setDragSourceActive(boolean state);
	public boolean setDropTargetActive(boolean state);

	/**
	 * This method returns the set of DND actions which are accepted by this
	 * Component.
	 * @see java.awt.dnd.DnDConstants
	 */
//	public int getDefaultDNDActions();

	/**
	 * This method returns the set of gestures which can be initiated by this
	 * Component;
	 *
	 * @see java.awt.dnd.DnDConstants
	 */
//	public int getDragGestures();

	/**
	 * This method returns the set of DataFlavors accepted by this component.
	 * These DataFlavors must be chosen from those implemented by the class
	 * DragNDropTool getDataFlavors()
	 *
	 * @see DragNDropTool#getDataFlavors
	 * @see java.awt.datatransfer.DataFlavor
	 */
//	public DataFlavor[] getTransferDataFlavors();

	/**
	 * This method returns true if the parameter flavor is a supported
	 * DataFlavor.  A DataFlavor is a different form of the same Data.  Two
	 * seperate components could receive the same Data but in different
	 * DataFlavors depending on what they implement that the formats the
	 * Transferable can provide from its contained data.
	 */
//	public boolean isDataFlavorSupported(DataFlavor flavor);
	/**
	 * When data is received which implements one of the acceptable DataFlavors
	 * for this component and is an acceptable drop action, the Transferable
	 * which contains the data of the drop will be passed into this function.
	 * If successfully processed, the user should return true.  return false
	 * otherwise.
	 */
//	public boolean acceptDrop(Transferable trans);
}