/*
 * @(#)DNDHelper.java
 *
 * Project:		JHotdraw - a GUI framework for technical drawings
 *				http://www.jhotdraw.org
 *				http://jhotdraw.sourceforge.net
 * Copyright:	© by the original author(s) and all contributors
 * License:		Lesser GNU Public License (LGPL)
 *				http://www.opensource.org/licenses/lgpl-license.html
 */

package CH.ifa.draw.contrib.dnd;

import java.awt.dnd.*;
import java.awt.*;
import java.util.*;
import CH.ifa.draw.framework.*;
import java.awt.datatransfer.*;
import java.io.*;
import javax.swing.JComponent;

/**
 * @author  C.L.Gilbert <dnoyeb@sourceforge.net>
 * @version <$CURRENT_VERSION$>
 */
public abstract class DNDHelper implements DropTargetListener,DragSourceListener,DragGestureListener {
	public static DataFlavor ASCIIFlavor = new DataFlavor("text/plain; charset=ascii", "ASCII text");
	protected DragGestureRecognizer dgr;
	protected DropTarget dt;
	protected Boolean autoscrollState;

	abstract protected DrawingView view();

	protected static Object ProcessReceivedData(DataFlavor flavor, Transferable transferable) {
		if (transferable == null) {
			return null;
		}
		try {
		    if (flavor.equals(DataFlavor.stringFlavor)) {
				String str = (String) transferable.getTransferData(DataFlavor.stringFlavor);
				return str;
			}
			else if (flavor.equals(DataFlavor.javaFileListFlavor)) {
				java.util.List aList = (java.util.List)transferable.getTransferData(DataFlavor.javaFileListFlavor);
				File fList [] = new File[aList.size()];
				aList.toArray(fList);
				return fList;
			}
			else if (flavor.equals(ASCIIFlavor)) {
				String txt = null;
				/* this may be too much work for locally received data */
				InputStream is = (InputStream)transferable.getTransferData(ASCIIFlavor);
				int length = is.available();
				byte[] bytes = new byte[length];
				int n = is.read(bytes);
				if (n > 0) {
					/* seems to be a 0 tacked on the end of Windows strings.  I
					 * havent checked other platforms.  This does not happen
					 * with windows socket io.  strange?
					 */
					//for (int i = 0; i < length; i++) {
					//    if (bytes[i] == 0) {
					//        length = i;
					//        break;
					//    }
					//}
					txt = new String(bytes, 0, n);
				}
				return txt;
			}
			else if (flavor.equals(DNDFiguresTransferable.DNDFiguresFlavor)) {
				DNDFigures ff = (DNDFigures) transferable.getTransferData(DNDFiguresTransferable.DNDFiguresFlavor);
				return ff;
			}
			else {
				return null;
			}
		}
		catch (java.io.IOException ioe) {
			System.err.println(ioe);
			return null;
		}
		catch (UnsupportedFlavorException ufe) {
			System.err.println(ufe);
			return null;
		}
		catch (ClassCastException cce) {
			System.err.println(cce);
			return null;
		}
	}

	public boolean setDropTargetActive(boolean state) {
		if (state == true) {
		    return createDropTarget();
		}
		else {
			destroyDropTarget();
			return false;
		}
	}

	public boolean setDragSourceActive(boolean state) {
		if (state == true) {
			return createDragGestureRecognizer();
		}
		else {
			destroyDragGestreRecognizer();
			return false;
		}
	}

	protected boolean createDropTarget() {
		if (Component.class.isInstance(view())) {
			try {
				dt = new DropTarget((Component)view(), DnDConstants.ACTION_COPY_OR_MOVE, this);
				System.out.println( "" + view() + " Initialized to DND.");
				return true;
			}
			catch (java.lang.NullPointerException npe) {
				System.err.println("View Failed to initialize to DND.");
				System.err.println("Container likely did not have peer before the DropTarget was added");
				System.err.println(npe);
				npe.printStackTrace();
			}
		}
		return false;
	}
	protected void destroyDropTarget() {
		if (dt!= null) {
			dt.setComponent(null);
			dt.removeDropTargetListener(this);
			dt = null;
		}
	}


	/**
	 * Used to create the gesture recognizer which in effect turns on draggability.
	 */
	protected boolean createDragGestureRecognizer() {
		if (Component.class.isInstance(view())) {
			Component c = (Component)view();
			dgr =	DragSource.getDefaultDragSource().createDefaultDragGestureRecognizer(
					c,
					DnDConstants.ACTION_COPY_OR_MOVE,
					this);
			//System.out.println("DragGestureRecognizer created: " + dgl);
			return true;
		}
		else
			return false;
	}

	/**
	 * Used to destroy the gesture listener which ineffect turns off dragability.
	 */
	protected void destroyDragGestreRecognizer() {
		System.out.println("Destroying DGR");
		if (dgr != null) {
			dgr.removeDragGestureListener(this);
	    	dgr.setComponent(null);
			dgr = null;
		}
	}

	/*******************************************DragGestureListener*****************/

	/**
	 * This function is called when the drag action is detected.  If it agrees
	 * with the attempt to drag it calls startDrag(), if not it does nothing.
	 */
	public void dragGestureRecognized(DragGestureEvent dge) {
		Component c = dge.getComponent();
		//System.out.println("recognized for " + c);

		if (DrawingView.class.isInstance(c)) {
			boolean found = false;
			DrawingView dv = (DrawingView)c;
			/* Send the drawing view which inspired the action a mouseUp to clean
			up its current tool.  THis is because mouse up will otherwise never
			be send and the tool will be stuck with only mouse down which means
			it will likely stay activated.  solve later for not just make
			but report. */
			/* this is a list of cloned figures */
			FigureEnumeration selectedElements = dv.selection();

			if (selectedElements.hasNextFigure() == false) {
				return;
			}

			Point p = dge.getDragOrigin();
//				System.out.println("origin at " + p);
			while (selectedElements.hasNextFigure()) {
				Figure f = selectedElements.nextFigure();
				if (f.containsPoint(p.x, p.y)) {
/*              Rectangle r = figgy.displayBox();
					sx = r.width;
					sy = r.height;*/
					//System.out.println("figure is " + figgy);
					found = true;
					break;
				}
			}
			if (found == true) {
				DNDFigures dndff = new DNDFigures(dv.selection(), p);
				DNDFiguresTransferable trans = new DNDFiguresTransferable(dndff);

				/* SAVE FOR FUTURE DRAG IMAGE SUPPORT */
				/* drag image support that I need to test on some supporting platform.
				windows is not supporting this on NT so far. Ill test 98 and 2K next

				boolean support = dragSource.isDragImageSupported();
				java.awt.image.BufferedImage  bi = new BufferedImage(sx,sy,BufferedImage.TYPE_INT_RGB);
				Graphics2D g = bi.createGraphics();
				Iterator itr2 = selectedElements.iterator();
				while ( itr2.hasNext() ) {
					Figure fig = (Figure) itr2.next();
					fig = (Figure)fig.clone();
					Rectangle rold = fig.displayBox();
					fig.moveBy(-rold.x,-rold.y);
					fig.draw(g);
				}
				g.setBackground(Color.red);
				dge.getDragSource().startDrag(
								dge,
								DragSource.DefaultMoveDrop,
								bi,
								new Point(0,0),
								trans,
								this);
				*/
				if (JComponent.class.isInstance( c )) {
				    ((JComponent)c).setAutoscrolls(false);
				}
				dge.getDragSource().startDrag(
								dge,
								null,
								trans,
								this);
			}
		}
	}
	/***************************************End DragGestureListener*****************/

	/*******************************************DropTargetListener*****************/
	private int     fLastX=0, fLastY=0;      // previous mouse position

	/**
	 * Called when a drag operation has encountered the DropTarget.
	 */
	public void dragEnter(DropTargetDragEvent dtde) {
		//System.out.println("DropTargetDragEvent-dragEnter");
		supportDropTargetDragEvent(dtde);
		if (fLastX == 0) {
			fLastX = dtde.getLocation().x;
		}
		if (fLastY == 0) {
			fLastY = dtde.getLocation().y;
		}
	}

	/**
	 * The drag operation has departed the DropTarget without dropping.
	 */
	public void dragExit(DropTargetEvent dte) {
		//System.out.println("DropTargetEvent-dragExit");
	}

	/**
	 * Called when a drag operation is ongoing on the DropTarget.
	 */
	 public void dragOver(DropTargetDragEvent dtde) {
		//System.out.println("DropTargetDragEvent-dragOver");
		if (supportDropTargetDragEvent(dtde)==true) {
			int x=dtde.getLocation().x;
			int y=dtde.getLocation().y;
			if ((Math.abs(x - fLastX) > 0) || (Math.abs(y - fLastY) > 0) ) {
				//FigureEnumeration fe = view().selectionElements();
				//while (fe.hasNextFigure()) {
				//	fe.nextFigure().moveBy(x - fLastX, y - fLastY);
				//	System.out.println("moving Figures " + view());
				//}
				//view().checkDamage();
				fLastX = x;
				fLastY = y;
			}
		}
	 }

	/**
	 * The drag operation has terminated with a drop on this DropTarget.
	 */
	public void drop(DropTargetDropEvent dtde) {
		System.out.println("DropTargetDropEvent-drop");

		if (dtde.isDataFlavorSupported(DNDFiguresTransferable.DNDFiguresFlavor) == true) {
			System.out.println("DNDFiguresFlavor");
			if ((dtde.getDropAction() & DnDConstants.ACTION_COPY_OR_MOVE) != 0 ) {
				System.out.println("copy or move");
				if (dtde.isLocalTransfer() == false) {
					System.err.println("Intra-JVM Transfers not implemented for figures yet.");
					return;
				}
				dtde.acceptDrop(dtde.getDropAction());
				try { /* protection from a malicious dropped object */
					DNDFigures ff = (DNDFigures)ProcessReceivedData(DNDFiguresTransferable.DNDFiguresFlavor, dtde.getTransferable());
					FigureEnumeration fe = ff.getFigures();
					Point theO = ff.getOrigin();
					view().clearSelection();
					Point newP = dtde.getLocation();
					/** origin is where the figure thinks it is now
					  * newP is where the mouse is now.
					  * we move the figure to where the mouse is with this equation
					  */
					int dx = newP.x - theO.x;  /* distance the mouse has moved */
					int dy = newP.y - theO.y;  /* distance the mouse has moved */
					//System.out.println("mouse at " + newP);
					while (fe.hasNextFigure()) {
						Figure f = fe.nextFigure();
						//System.out.println("figure location = " + f.displayBox());

						//    f.moveBy(newP.x - fLastX , newP.y - fLastY);

						f.moveBy(dx , dy);
						//System.out.println("figure new location = " + f.displayBox());
						view().add(f);
						if (dtde.getDropAction() == DnDConstants.ACTION_MOVE)
							view().addToSelection(f);
						System.out.println("added to view");
					}
					view().checkDamage();
					dtde.getDropTargetContext().dropComplete(true);
				}
				catch (NullPointerException npe) {
					npe.printStackTrace();
					dtde.getDropTargetContext().dropComplete(false);
				}
			}
			else {
				dtde.rejectDrop();
			}
		}
		else if (dtde.isDataFlavorSupported(DataFlavor.stringFlavor)) {
			//System.out.println("String flavor dropped.");
			dtde.acceptDrop(dtde.getDropAction());
			Object o = ProcessReceivedData(DataFlavor.stringFlavor, dtde.getTransferable());
			if (o != null) {
				//System.out.println("Received string flavored data.");
				dtde.getDropTargetContext().dropComplete(true);
			}
			else {
				dtde.getDropTargetContext().dropComplete(false);
			}
		}
		else if (dtde.isDataFlavorSupported(ASCIIFlavor) == true) {
			//System.out.println("ASCII Flavor dropped.");
			dtde.acceptDrop(DnDConstants.ACTION_COPY);
			Object o = ProcessReceivedData(ASCIIFlavor, dtde.getTransferable());
			if (o!= null) {
				//System.out.println("Received ASCII Flavored data.");
				dtde.getDropTargetContext().dropComplete(true);
				//System.out.println(o);
			}
			else {
				dtde.getDropTargetContext().dropComplete(false);
			}
		}
		else if (dtde.isDataFlavorSupported(DataFlavor.javaFileListFlavor)) {
			//System.out.println("Java File List Flavor dropped.");
			dtde.acceptDrop(DnDConstants.ACTION_COPY);
			java.io.File [] fList = (java.io.File[]) ProcessReceivedData(DataFlavor.javaFileListFlavor, dtde.getTransferable());
			if (fList != null) {
				//System.out.println("Got list of files.");
				for (int x=0; x< fList.length; x++ ) {
					System.out.println(fList[x].getAbsolutePath());
				}
				dtde.getDropTargetContext().dropComplete(true);
			}
			else {
				dtde.getDropTargetContext().dropComplete(false);
			}
		}
		fLastX = 0;
		fLastY = 0;
	}

	/**
	 * Called if the user has modified the current drop gesture.
	 */
	public void dropActionChanged(DropTargetDragEvent dtde) {
		//System.out.println("DropTargetDragEvent-dropActionChanged");
		supportDropTargetDragEvent(dtde);
	}

	/**
	 * Tests wether the Drag event is of a type that we support handling
	 * Check the DND interface and support the events it says it supports
	 * if not a dnd interface comp, then dont support! because we dont even
	 * really know what kind of view it is.
	 */
	protected boolean supportDropTargetDragEvent(DropTargetDragEvent dtde) {
		if (dtde.isDataFlavorSupported(DNDFiguresTransferable.DNDFiguresFlavor) == true) {
			if (dtde.getDropAction() == DnDConstants.ACTION_COPY) {
				dtde.acceptDrag(DnDConstants.ACTION_COPY);
				return true;
			}
			else if (dtde.getDropAction() == DnDConstants.ACTION_MOVE) {
				dtde.acceptDrag(DnDConstants.ACTION_MOVE);
				return true;
			}
			else {
				dtde.rejectDrag();
				return false;
			}
		}
		else if (dtde.isDataFlavorSupported(ASCIIFlavor) == true) {
			dtde.acceptDrag(dtde.getDropAction());
			return true;
		}
		else if (dtde.isDataFlavorSupported(DataFlavor.stringFlavor) == true) {
			dtde.acceptDrag(dtde.getDropAction());
			return true;
		}
		else if (dtde.isDataFlavorSupported(DataFlavor.javaFileListFlavor) == true) {
			dtde.acceptDrag(dtde.getDropAction());
			return true;
		}
		else {
			dtde.rejectDrag();
			return false;
		}
	}
	/***************************************End DropTargetListener*****************/

	/************************************ Begin DragSourceListener*****************/

	/**
	 * This method is invoked to signify that the Drag and Drop operation is complete.
	 * This is the last method called in the process.
	 */
	public void dragDropEnd(DragSourceDropEvent dsde) {
		DrawingView view = (DrawingView) dsde.getDragSourceContext().getComponent();
		System.out.println("DragSourceDropEvent-dragDropEnd");
		if (dsde.getDropSuccess() == true) {
			if (dsde.getDropAction() == DnDConstants.ACTION_MOVE) {
//                System.out.println("DragSourceDropEvent-ACTION_MOVE");
				//get the flavor in order of ease of use here.
				DNDFigures df = (DNDFigures)ProcessReceivedData(DNDFiguresTransferable.DNDFiguresFlavor, dsde.getDragSourceContext().getTransferable());
				FigureEnumeration fe = df.getFigures();

				//how can fe be null?
				if (fe != null) {
					while (fe.hasNextFigure()) {
						//how can this work on cloned fe?
						Figure f = fe.nextFigure();
						view.remove(f);
						System.out.println("removing " + f);
					}
					view.clearSelection();
					view.checkDamage();
				}
			}
			else if (dsde.getDropAction() == DnDConstants.ACTION_COPY) {
//                System.out.println("DragSourceDropEvent-ACTION_COPY");
			}
		}

		if (autoscrollState != null) {
			Component c = dsde.getDragSourceContext().getComponent();
			if (JComponent.class.isInstance( c )) {
				JComponent jc = (JComponent)c;
				jc.setAutoscrolls(autoscrollState.booleanValue());
				autoscrollState= null;
			}
		}
	}
	/**
	 * Called as the hotspot enters a platform dependent drop site.
	 */
	public void dragEnter(DragSourceDragEvent dsde) {
		if (autoscrollState == null) {
			Component c = dsde.getDragSourceContext().getComponent();
			if (JComponent.class.isInstance( c )) {
				JComponent jc = (JComponent)c;
				autoscrollState= new Boolean(jc.getAutoscrolls());
				jc.setAutoscrolls(false);
			}
		}

		//System.out.println("DragSourceDragEvent-dragEnter");
//		dsde.getDragSourceContext().
	}
	/**
	 * Called as the hotspot exits a platform dependent drop site.
	 */
	public void dragExit(DragSourceEvent dse) {
	}
	/**
	 * Called as the hotspot moves over a platform dependent drop site.
	 */
	public void dragOver(DragSourceDragEvent dsde) {
		//System.out.println("DragSourceDragEvent-dragOver");
	}
	/**
	 * Called when the user has modified the drop gesture.
	 */
	public void dropActionChanged(DragSourceDragEvent dsde) {
	}
}
	/**
	 * These transferable objects are used to package your data when you want
	 * to initiate a transfer.  They are not used when you only want to receive
	 * data.  Formating the data is the responsibility of the sender primarily.
	 * Untested.  Used for dragging ASCII text out of JHotDraw
	 */
/*	public class ASCIIText implements Transferable
	{
		String s = new String("This is ASCII text");
		byte[] bytes;

		public DataFlavor[] getTransferDataFlavors() {
			return new DataFlavor[] { ASCIIFlavor };
		}

		public boolean isDataFlavorSupported(DataFlavor dataFlavor) {
			return dataFlavor.equals(ASCIIFlavor);
		}

		public Object getTransferData(DataFlavor dataFlavor)
			throws UnsupportedFlavorException, IOException  {
			if (!isDataFlavorSupported(dataFlavor))
						throw new UnsupportedFlavorException(dataFlavor);

			bytes = new byte[s.length() + 1];
			for (int i = 0; i < s.length(); i++)
				bytes = s.getBytes();
			bytes[s.length()] = 0;
			return new ByteArrayInputStream(bytes);
		}
	}*/