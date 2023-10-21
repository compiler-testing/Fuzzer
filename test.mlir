
module {
  func.func @main(%arg0: tensor<84x78x94x24xf32>) -> ( tensor<84x78x94x24xf32>) {
    %0 = tensor.empty() : tensor<84x78x94x24xf32>
    return %0:  tensor<84x78x94x24xf32>
  }
}