import { Link } from 'react-router-dom';
import type { ProductSummary } from '../../types/content';

interface ProductCardProps {
  product: ProductSummary;
}

export function ProductCard({ product }: ProductCardProps) {
  return (
    <div className="border border-border-subtle border-l-2 border-l-accent rounded-lg p-8 bg-surface-raised transition-colors duration-150 hover:border-accent/40">
      <h3 className="text-text-primary text-xl font-semibold">{product.name}</h3>
      <p className="text-accent text-sm font-medium mt-1">{product.tagline}</p>
      <p className="text-text-secondary text-sm leading-relaxed mt-4">
        {product.description}
      </p>

      <ul className="mt-6 space-y-2">
        {product.features.map((feature) => (
          <li
            key={feature}
            className="flex items-start gap-2 text-text-secondary text-sm"
          >
            <span className="text-accent mt-0.5 shrink-0">•</span>
            {feature}
          </li>
        ))}
      </ul>

      <Link
        to={product.href}
        className="inline-flex items-center mt-6 text-accent text-sm font-medium hover:text-accent-muted transition-colors duration-150"
      >
        Learn more <span className="ml-1">→</span>
      </Link>
    </div>
  );
}
