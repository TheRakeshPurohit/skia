               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %main "main"                  ; id %2

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %17 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1


               ; Function main
       %main = OpFunction %void None %11

         %12 = OpLabel
         %13 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %17 =   OpLoad %v4float %13                ; RelaxedPrecision
         %18 =   OpVectorShuffle %v3float %17 %17 0 1 2     ; RelaxedPrecision
         %20 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %22 =   OpLoad %v4float %20                ; RelaxedPrecision
         %23 =   OpVectorShuffle %v3float %22 %22 0 1 2     ; RelaxedPrecision
         %24 =   OpFAdd %v3float %18 %23                    ; RelaxedPrecision
         %26 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %27 =   OpLoad %v4float %26                ; RelaxedPrecision
         %28 =   OpVectorShuffle %v3float %27 %27 0 1 2     ; RelaxedPrecision
         %29 =   OpVectorTimesScalar %v3float %28 %float_2  ; RelaxedPrecision
         %30 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %31 =   OpLoad %v4float %30                ; RelaxedPrecision
         %32 =   OpVectorShuffle %v3float %31 %31 0 1 2     ; RelaxedPrecision
         %33 =   OpFMul %v3float %29 %32                    ; RelaxedPrecision
         %34 =   OpFSub %v3float %24 %33                    ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %34 0            ; RelaxedPrecision
         %36 =   OpCompositeExtract %float %34 1            ; RelaxedPrecision
         %37 =   OpCompositeExtract %float %34 2            ; RelaxedPrecision
         %38 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %39 =   OpLoad %v4float %38                ; RelaxedPrecision
         %40 =   OpCompositeExtract %float %39 3    ; RelaxedPrecision
         %42 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %43 =   OpLoad %v4float %42                ; RelaxedPrecision
         %44 =   OpCompositeExtract %float %43 3    ; RelaxedPrecision
         %45 =   OpFSub %float %float_1 %44         ; RelaxedPrecision
         %46 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %47 =   OpLoad %v4float %46                ; RelaxedPrecision
         %48 =   OpCompositeExtract %float %47 3    ; RelaxedPrecision
         %49 =   OpFMul %float %45 %48              ; RelaxedPrecision
         %50 =   OpFAdd %float %40 %49              ; RelaxedPrecision
         %51 =   OpCompositeConstruct %v4float %35 %36 %37 %50  ; RelaxedPrecision
                 OpStore %sk_FragColor %51
                 OpReturn
               OpFunctionEnd
