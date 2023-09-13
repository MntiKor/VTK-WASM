#include <vtkGraph.h>
#include <vtkMutableDirectedGraph.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkNew.h>

#include "vtkTestGraphAlgorithmFilter.h"

void TestDirected();
void TestUndirected();

int main(int, char*[])
{
  TestDirected();
  TestUndirected();

  return EXIT_SUCCESS;
}

void TestDirected()
{

  vtkNew<vtkMutableDirectedGraph> g;
  vtkIdType v1 = g->AddVertex();
  vtkIdType v2 = g->AddVertex();

  g->AddEdge(v1, v2);
  std::cout << "Input type: " << g->GetClassName() << std::endl;

  vtkNew<vtkTestGraphAlgorithmFilter> filter;
  filter->SetInputData(g);
  filter->Update();

  std::cout << "Output type: " << filter->GetOutput()->GetClassName()
            << std::endl;
  std::cout << "Output has " << filter->GetOutput()->GetNumberOfVertices()
            << " vertices." << std::endl;
  std::cout << std::endl;
}

void TestUndirected()
{
  std::cout << "TestUndirected" << std::endl;
  vtkNew<vtkMutableUndirectedGraph> g;
  vtkIdType v1 = g->AddVertex();
  vtkIdType v2 = g->AddVertex();

  g->AddEdge(v1, v2);
  std::cout << "Input type: " << g->GetClassName() << std::endl;

  vtkNew<vtkTestGraphAlgorithmFilter> filter;
  filter->SetInputData(g);
  filter->Update();

  std::cout << "Output type: " << filter->GetOutput()->GetClassName()
            << std::endl;
  std::cout << "Output has " << filter->GetOutput()->GetNumberOfVertices()
            << " vertices." << std::endl;
}
