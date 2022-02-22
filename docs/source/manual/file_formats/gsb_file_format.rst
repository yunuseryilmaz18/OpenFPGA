.. _file_formats_gsb_xml:

GSB- General Switch Box (.xml)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``write_gsb_to_xml`` command writes out the genreal switch box information in 
``.xml`` file format which allows designer to debug connectivity and connection 
related issue. If ``--unique`` option is enabled in the command, 
information related to each unique connection box and switch box is written out
in file a ``cb?_?__?_gsb.xml`` and ``sb_?__?_gsb.xml`` respectively.
Format of each file is detailed below.

Switch Box GSB (.xml)
---------------------
.. code-block:: xml
    
    <rr_gsb x="LOC_X" y="LOC_Y" TYPE="SB" num_sides="SIDE">
        <CHANY side="SIDE" index="INDX" node_id="nID" segment_id="sID" mux_size="mSIZE">
            <driver_node type="TYPE" side="SIDE" index="INDX" node_id="nID" grid_side="gSIDE"/>
            ..
        </CHANY>
        <INPUT CHANX="" CHANY="" OPIN="">
            <driver_node type="CHANX" side="SIDE" index="INDX" node_id="nID" segment_id="sID"/>
            <driver_node type="CHANY" side="SIDE" index="INDX" node_id="nID" segment_id="sID"/>
            <driver_node type="OPIN" side="SIDE" index="INDX" node_id="nID" grid_side="gSIDE"/>
        </INPUT>
    </rr_gsb>


.. option:: <rr_gsb x="LOC_X" y="LOC_Y" TYPE="SB" num_sides="SIDE">

  - ``LOC_X`` X coordinate of the switch box in 2D FPGA grid
  - ``LOC_Y`` Y coordinate of the switch box in 2D FPGA grid
  - ``mSIZE`` Number of input signals multiplexes to generate the given output channel
  - ``SIDE`` Not Sure

.. option:: <CHANY side="SIDE" index="INDX" node_id="nID" segment_id="sID" mux_size="SIZE">

  This block details each output channel from the given switch box, 
  CHANY tag represents it is a vertical output channel.
  Alternately this tag can be CHANX to represent horizontal channels

  - ``SIDE`` Direction where the output channel is located 
  - ``INDX`` Index of the output channel (Note: index are shared beetween incoming and outgoing channels on that side)
  - ``nID`` Internal node ID unique for the given output channel 
  - ``sID`` Not Sure

.. option:: <driver_node type="OPIN" side="SIDE" index="INDX" node_id="nID" grid_side="gSIDE"/>
    <driver_node type="CHANX" side="SIDE" index="INDX" node_id="nID" segment_id="sID"/>

  This set declarations detals inputs signals to the current output driver.

  - ``type`` Details type of connection, it can be  ``OPIN``, ``CHANX`` or ``CHANY``
  - ``SIDE`` Side of the incoming connection
  - ``INDX`` Index of the incoming channel/connection
  - ``nID`` Internal unique node ID assigned to this node
  - ``sID`` Not sure
  - ``gSIDE`` Applicable only if ``TYPE`` is ``OPIN``, it represents from which direction ``OPIN`` exits from ``LB``

.. option:: <INPUT CHANX="CHANX_COUNT" CHANY="CHANY_COUNT" OPIN="OPIN_PIN">

  - ``CHANX_COUNT`` Number of CHANX inputs
  - ``CHANY_COUNT`` Number of CHANY inputs
  - ``OPIN_PIN`` Number of OPIN inputs 

Connection Box GSB (.xml)
-------------------------
.. code-block:: xml
    
    <rr_gsb x="LOC_X" y="LOC_Y" TYPE="TYPE" num_sides="SIDE">
        <IPIN side="SIDE" index="INDX" node_id="nID" mux_size="nSIZE">
            <driver_node type="TYPE" side="SIDE" node_id="nID" index="INDX" segment_id="sID"/>
        </IPIN>
        <INPUT CHANY="">
            <driver_node type="CHANX" side="SIDE" index="INDX" node_id="nID" segment_id="sID"/>
        </INPUT>
    </rr_gsb>

.. option:: <rr_gsb x="LOC_X" y="LOC_Y" TYPE="TYPE" num_sides="SIDE">
  
  - ``LOC_X`` X coordinate of the switch box in 2D FPGA grid
  - ``LOC_Y`` Y coordinate of the switch box in 2D FPGA grid
  - ``TYPE`` ``CBX`` or ``CBY`` connection box
  - ``mSIZE`` Number of input signals multiplexes to generate the given output channel
  - ``SIDE`` Not Sure


.. option:: <IPIN side="SIDE" index="INDX" node_id="nID" mux_size="nSIZE">

  This represent output of the connection box.

  - ``SIDE`` Output pin direction 
  - ``INDX`` Index of the output pins
  - ``nID`` unique node id
  - ``mux_size`` Number of input signals multiplexes to generate the given output pin

.. option:: <driver_node type="TYPE" side="SIDE" node_id="nID" index="INDX" segment_id="sID"/>

  - ``TYPE`` Type of connections ``CHANX`` or ``CHANY`` channels
  - ``SIDE`` Direction of the channel. ``left`` or ``right`` if type is ``CHANX`` and ``top`` or ``bottom`` if type is ``CHANY``
  - ``INDX`` Index of the output pins
  - ``nID`` unique node id
  - ``SIDE`` Not Sure
