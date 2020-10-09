# Install libraries
#install.packages("dplyr")
#install.packages("ggplot2")

# Load libraries
library("dplyr", warn.conflicts = FALSE)
library("ggplot2")

# Load the data
dataframe = read.csv("sample-2-10_sorted.bam_filtered.snpt.cropped_RLE.csv")

# Do not generate Rplots.pdf
pdf(NULL)

# Color blind-friendly palette
cbPalette <- c("#56B4E9", "#009E73", "#999999", "#E69F00", "#F0E442", "#0072B2", "#D55E00", "#CC79A7")

# Compression ratio over encoding time
ggplot2::ggplot(dataframe, aes(x = total_encoding_time)) +
  ggplot2::geom_line(aes(y = current_encoded_ratio, color = "tomato")) +
  ggplot2::geom_line(aes(y = best_encoded_ratio, color = "green")) +
  ggplot2::theme_light() +
  ggplot2::theme(legend.position = "none") +
  ggplot2::labs(x = "Encoding time [s]", y = "Compression ratio [%]") +
  ggplot2::ggsave(file = "compression-ratio-over-encoding-time.png", width = 15, height = 8, units = "cm") +
  ggplot2::ggsave(file = "compression-ratio-over-encoding-time.pdf", width = 15, height = 8, units = "cm")
