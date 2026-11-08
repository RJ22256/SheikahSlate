#!/usr/bin/env python3
# (WSword) OpenDLSS-NR's demo pass made to run on WSword's frame (CI, before wsword_nr.dll is built: .github/workflows/
# build-windows.yml). Usage: patch_open_dlss.py <open-dlss checkout>. Every change must apply exactly once.
#
#   demo/nr_pass.cpp       - the motion: WSword's own (rgba16f, current -> previous in uv units, z = has a history:
#                            exec.c nr_apply makes it from the scene depth and the camera), copied in where the demo
#                            unpacked Filament's velocity buffer; no unpack pipeline.
#   demo/shaders/nr_composite.comp - the output: the result sRGB-encoded as it is. BOTW's image is already tone mapped
#                            (display-referred, linear 0..1); the demo's WebGI display transform (ACES fit, exposure
#                            1 / 0.6) is for a renderer's HDR scene radiance and tone mapped BOTW a second time.
import sys
from pathlib import Path

root = Path(sys.argv[1])


def patch(rel, edits):
    p = root / rel
    s = p.read_text(encoding="utf-8")
    for old, new in edits:
        n = s.count(old)
        if n != 1:
            sys.exit(f"patch_open_dlss: {rel}: {n} matches for {old[:70]!r}")
        s = s.replace(old, new)
    p.write_text(s, encoding="utf-8", newline="\n")
    print(f"patched {rel} ({len(edits)} edits)")


patch("demo/nr_pass.cpp", [
    ('  unpackPipeline_ = computePipeline(demoShaderDir + "/velocity_unpack.comp.spv", unpackLayout_);\n',
     '  // (WSword) no unpack: the motion comes made (record)\n'),
    ('  barrier(commands, velocity_.image, (VkImageLayout)velocity.layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,\n'
     '          VK_ACCESS_MEMORY_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);\n',
     '  barrier(commands, velocity_.image, (VkImageLayout)velocity.layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,\n'
     '          VK_ACCESS_MEMORY_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT);\n'),
    ('''  vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_COMPUTE, unpackPipeline_);
  vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE, unpackLayout_, 0, 1, &unpackSet_, 0, nullptr);
  UnpackPush push{};
  memcpy(push.background, frame.background, sizeof(push.background));
  push.width = width_; push.height = height_;
  vkCmdPushConstants(commands, unpackLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
  vkCmdDispatch(commands, (width_ + 7) / 8, (height_ + 7) / 8, 1);
''', '''  // (WSword) the motion as WSword made it (rgba16f, this size): copied into the motion image the passes read
  barrier(commands, sceneMotion_.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
          VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
  {
    VkImageCopy motionCopy{};
    motionCopy.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    motionCopy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    motionCopy.extent = {width_, height_, 1};
    vkCmdCopyImage(commands, velocity_.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, sceneMotion_.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &motionCopy);
  }
  barrier(commands, sceneMotion_.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
          VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT);
'''),
    # (the queue the network shares with WSword, not the device: WSword's Lossless frame generation presents on a
    # second queue from a thread of its own)
    ('void NrPass::destroySized() {\n  VK_CHECK(vkDeviceWaitIdle(device_));',
     'void NrPass::destroySized() {\n  VK_CHECK(vkQueueWaitIdle(context_->queue()));'),
])

patch("demo/shaders/nr_composite.comp", [
    ('vec3 webgiDisplay(vec3 scene) {',
     '// (WSword) the display value of an already tone mapped (display-referred, linear) colour: its sRGB encoding\n'
     'vec3 wswordDisplay(vec3 c) {\n'
     '  c = clamp(c, vec3(0.0), vec3(1.0));\n'
     '  return vec3(srgbEncode(c.r), srgbEncode(c.g), srgbEncode(c.b));\n'
     '}\n\n'
     'vec3 webgiDisplay(vec3 scene) {'),
    ('imageStore(outputImage, ivec2(id), vec4(webgiDisplay(original * paper), 1.0));',
     'imageStore(outputImage, ivec2(id), vec4(wswordDisplay(original * paper), 1.0));'),
    ('imageStore(outputImage, ivec2(id), vec4(webgiDisplay(result), 1.0));',
     'imageStore(outputImage, ivec2(id), vec4(wswordDisplay(result), 1.0));'),
])
