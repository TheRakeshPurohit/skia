               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %outParameterWrite_vh4 "outParameterWrite_vh4"    ; id %2
               OpName %outParameterWriteIndirect_vh4 "outParameterWriteIndirect_vh4"    ; id %3
               OpName %inoutParameterWrite_vh4 "inoutParameterWrite_vh4"                ; id %4
               OpName %inoutParameterWriteIndirect_vh4 "inoutParameterWriteIndirect_vh4"    ; id %5
               OpName %main "main"                                                          ; id %6
               OpName %c "c"                                                                ; id %52

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %26 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %c RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %25 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %49 = OpTypeFunction %v4float %_ptr_Function_v2float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function outParameterWrite_vh4
%outParameterWrite_vh4 = OpFunction %void None %25
         %26 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %27 = OpLabel
         %28 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %28                ; RelaxedPrecision
                 OpStore %26 %32
                 OpReturn
               OpFunctionEnd


               ; Function outParameterWriteIndirect_vh4
%outParameterWriteIndirect_vh4 = OpFunction %void None %25
         %33 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %34 = OpLabel
         %35 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %36 =   OpFunctionCall %void %outParameterWrite_vh4 %35
         %37 =   OpLoad %v4float %35                ; RelaxedPrecision
                 OpStore %33 %37
                 OpReturn
               OpFunctionEnd


               ; Function inoutParameterWrite_vh4
%inoutParameterWrite_vh4 = OpFunction %void None %25
         %38 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %39 = OpLabel
         %40 =   OpLoad %v4float %38                ; RelaxedPrecision
         %41 =   OpLoad %v4float %38                ; RelaxedPrecision
         %42 =   OpFMul %v4float %40 %41            ; RelaxedPrecision
                 OpStore %38 %42
                 OpReturn
               OpFunctionEnd


               ; Function inoutParameterWriteIndirect_vh4
%inoutParameterWriteIndirect_vh4 = OpFunction %void None %25
         %43 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %44 = OpLabel
         %46 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %45 =   OpLoad %v4float %43                            ; RelaxedPrecision
                 OpStore %46 %45
         %47 =   OpFunctionCall %void %inoutParameterWrite_vh4 %46
         %48 =   OpLoad %v4float %46                ; RelaxedPrecision
                 OpStore %43 %48
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %49         ; RelaxedPrecision
         %50 = OpFunctionParameter %_ptr_Function_v2float

         %51 = OpLabel
          %c =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %53 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %56 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %59 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %62 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %54 =   OpFunctionCall %void %outParameterWrite_vh4 %53
         %55 =   OpLoad %v4float %53                ; RelaxedPrecision
                 OpStore %c %55
         %57 =   OpFunctionCall %void %outParameterWriteIndirect_vh4 %56
         %58 =   OpLoad %v4float %56                ; RelaxedPrecision
                 OpStore %c %58
                 OpStore %59 %58
         %60 =   OpFunctionCall %void %inoutParameterWrite_vh4 %59
         %61 =   OpLoad %v4float %59                ; RelaxedPrecision
                 OpStore %c %61
                 OpStore %62 %61
         %63 =   OpFunctionCall %void %inoutParameterWriteIndirect_vh4 %62
         %64 =   OpLoad %v4float %62                ; RelaxedPrecision
                 OpStore %c %64
                 OpReturnValue %64
               OpFunctionEnd
