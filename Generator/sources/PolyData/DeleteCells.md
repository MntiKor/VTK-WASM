### Description

This example demonstrates how to delete a cell from a vtkPolyData. This is a three-step process: first, build upward-links from points to cells; second, mark cells for deletion; third, delete the marked cells. The first step is necessary to create a data structure suitable for efficient delete operations. Creating this data structure is computationally expensive, so it is only carried out before operations that need it, e.g. those that require changes to the topology.
